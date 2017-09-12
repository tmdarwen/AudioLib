#include <Application/PhaseVocoderMediator.h>
#include <Application/Transients.h>
#include <WaveFile/WaveFileReader.h>
#include <WaveFile/WaveFileWriter.h>
#include <Signal/PhaseVocoder.h>
#include <Signal/Resampler.h>
#include <Utilities/Exception.h>
#include <iostream>
#include <cmath>

PhaseVocoderMediator::PhaseVocoderMediator(const PhaseVocoderSettings& settings) : settings_{settings}
{

}

PhaseVocoderMediator::~PhaseVocoderMediator()
{

}

void PhaseVocoderMediator::Process()
{
	InstantiateWaveFileObjects();

	InstantiateResampler();

	if(settings_.StretchFactorGiven() || settings_.PitchShiftValueGiven() || settings_.TransientCallbackGiven())
	{
		// Even if we don't stretch the audio, and are just pitch shifting, the pitch shifter requires 
		// stretching, and if we're stretching, we want to properly handle transients so the audio 
		// quality doesn't suffer.
		ObtainTransients();
	}

	// If we're not stretching, pitch shifting or resampling the audio, we must have only been 
	// displaying transients (via the transient callback) so we're done processing at this point.
	if(!settings_.StretchFactorGiven() && !settings_.PitchShiftValueGiven() && !settings_.ResampleValueGiven())
	{
		return;
	}

	// We process audio by transient sections.  If no stretching of audio is happening (i.e. we're just  
	// resampling), we just treat the audio as one single transient section.
	if(settings_.StretchFactorGiven() || settings_.PitchShiftValueGiven() || settings_.TransientCallbackGiven())
	{
		// This handles any leading silence given in the input.
		HandleLeadingSilence();

		std::size_t transientStartIndex{0};
		auto transientPositions{transients_->GetTransients()};
		while(transientStartIndex < (transientPositions.size() - 1))
		{
			ProcessAudioSection(transientPositions[transientStartIndex], transientPositions[transientStartIndex + 1]);	
			++transientStartIndex;
		}

		// Handle the last transient section to the end of input
		ProcessAudioSection(transientPositions[transientStartIndex], waveReader_->GetSampleCount());
	}
	else
	{
		// If we're here, we are just resampling the audio.  No transients, no stretching.
		ProcessAudioSection(0, waveReader_->GetSampleCount());
	}

	// Flush the Resampler (if we're using it)
	if(settings_.ResampleValueGiven() || settings_.PitchShiftValueGiven())
	{
		auto audioData{resampler_->FlushAudioData()};
		waveWriter_->AppendAudioData(audioData.GetData());
	}
}

void PhaseVocoderMediator::ObtainTransients()
{
	TransientSettings transientSettings;

	if(settings_.InputWaveFileGiven())
	{
		transientSettings.SetInputWaveFile(settings_.GetInputWaveFile());
	}

	if(settings_.TransientCallbackGiven())
	{
		transientSettings.SetTransientCallback(settings_.GetTransientCallback());
	}

	if(settings_.TransientConfigFilenameGiven())
	{
		transientSettings.SetTransientConfigFilename(settings_.GetTransientConfigFilename());
	}

	transientSettings.SetTransientValleyToPeakRatio(settings_.GetValleyToPeakRatio());

	transients_.reset(new Transients(transientSettings));
	transients_->GetTransients();
}

void PhaseVocoderMediator::HandleLeadingSilence()
{
	auto transientPositions{transients_->GetTransients()};

	if(transientPositions.size() == 0)
	{
		HandleSilenceInInput(waveReader_->GetSampleCount());
	}
	else if(transientPositions[0] != 0)
	{
		HandleSilenceInInput(transientPositions[0]);
	}
}

AudioData PhaseVocoderMediator::GetAudioInput(std::size_t startSample, std::size_t length)
{
	return waveReader_->GetAudioData(startSample, length);
}

void PhaseVocoderMediator::HandleSilenceInInput(std::size_t sampleCount)
{
	std::size_t samplesToOutput{static_cast<std::size_t>(static_cast<double>(sampleCount) * settings_.GetStretchFactor() + 0.5)};

	std::size_t currentSamplePosition{0};
	while(currentSamplePosition < samplesToOutput)
	{
		std::size_t currentWriteAmount{bufferSize_};
		if(currentSamplePosition + bufferSize_ > samplesToOutput)
		{
			currentWriteAmount = samplesToOutput - currentSamplePosition;
		}

		AudioData silentAudioData;
		silentAudioData.AddSilence(currentWriteAmount);

		waveWriter_->AppendAudioData(silentAudioData.GetData());

		currentSamplePosition += currentWriteAmount;
	}
}

void PhaseVocoderMediator::ProcessAudioSection(std::size_t startSamplePosition, std::size_t endSamplePosition)
{
	std::size_t totalSamplesToRead{endSamplePosition - startSamplePosition};

	InstantiatePhaseVocoder(endSamplePosition - startSamplePosition);
	samplesOutputFromCurrentPhaseVocoder_ = 0;

	std::size_t currentSamplePosition{0};
	while(currentSamplePosition < totalSamplesToRead)
	{
		std::size_t samplesToRead{bufferSize_};
		if(currentSamplePosition + bufferSize_ > totalSamplesToRead)
		{
			samplesToRead = totalSamplesToRead - currentSamplePosition;
		}

		auto audioInputData{GetAudioInput(startSamplePosition + currentSamplePosition, samplesToRead)};
		ProcessInput(audioInputData);

		currentSamplePosition += samplesToRead;
	}

	FinalizeAudioSection(totalSamplesToRead);
}

void PhaseVocoderMediator::ProcessInput(const AudioData& audioInputData)
{
	AudioData resultingAudio;

	if(settings_.PitchShiftValueGiven() || (settings_.StretchFactorGiven() && settings_.ResampleValueGiven()))
	{
		resultingAudio = ProcessAudioWithResampler(ProcessAudioWithPhaseVocoder(audioInputData));
	}
	else if(settings_.StretchFactorGiven() && !settings_.PitchShiftValueGiven())
	{
		resultingAudio = ProcessAudioWithPhaseVocoder(audioInputData);
	}
	else if(settings_.ResampleValueGiven() && !settings_.PitchShiftValueGiven())
	{
		resultingAudio = ProcessAudioWithResampler(audioInputData);
	}
	else
	{
		Utilities::ThrowException("PhaseVocoderMediator has no action to perform");
	}

	waveWriter_->AppendAudioData(resultingAudio.GetData());
}

void PhaseVocoderMediator::FinalizeAudioSection(std::size_t totalInputSamples)
{
	AudioData audioData;

	if(settings_.StretchFactorGiven() || settings_.PitchShiftValueGiven())
	{
		std::size_t totalOutputSamplesNeeded{static_cast<std::size_t>(totalInputSamples * phaseVocoder_->GetStretchFactor() + 0.5)};
		std::size_t samplesStillNeeded{totalOutputSamplesNeeded - samplesOutputFromCurrentPhaseVocoder_};

		audioData = FlushPhaseVocoderOutput(samplesStillNeeded);
	}

	if(audioData.GetSize() && (settings_.ResampleValueGiven() || settings_.PitchShiftValueGiven()))
	{
		resampler_->SubmitAudioData(audioData);
	}
	else
	{
		waveWriter_->AppendAudioData(audioData.GetData());
	}
}

AudioData PhaseVocoderMediator::ProcessAudioWithPhaseVocoder(const AudioData& audioInputData)
{
	phaseVocoder_->SubmitAudioData(audioInputData);

	AudioData dataToReturn;

	while(phaseVocoder_->OutputSamplesAvailable())
	{
		std::size_t samplesToRetrieve{bufferSize_};
		if(samplesToRetrieve > phaseVocoder_->OutputSamplesAvailable())
		{
			samplesToRetrieve = phaseVocoder_->OutputSamplesAvailable();
		}

		dataToReturn.Append(phaseVocoder_->GetAudioData(samplesToRetrieve));
	}

	// If transient overlap data exist, mix it with this output
	if(transientSectionOverlap_.GetSize() && (dataToReturn.GetSize() >= transientSectionOverlap_.GetSize()))
	{
		dataToReturn = LinearCrossfade(transientSectionOverlap_, dataToReturn);
		transientSectionOverlap_.Clear();	
	}

	samplesOutputFromCurrentPhaseVocoder_ += dataToReturn.GetSize();

	return dataToReturn;
}

AudioData PhaseVocoderMediator::FlushPhaseVocoderOutput(std::size_t samplesNeeded)
{
	AudioData audioToReturn;
	auto flushedOutput{phaseVocoder_->FlushAudioData()};

	if(samplesNeeded)
	{
		if(samplesNeeded > flushedOutput.GetSize())
		{
			Utilities::ThrowException("Flushed output has less samples than still needed", samplesNeeded, flushedOutput.GetSize(), waveWriter_->GetSampleCount());
		}

		audioToReturn = flushedOutput.RetrieveRemove(samplesNeeded);
  
		// If transient overlap data exist, mix it with this output
		if(transientSectionOverlap_.GetSize())
		{
			audioToReturn = LinearCrossfade(transientSectionOverlap_, audioToReturn);
			transientSectionOverlap_.Clear();	
		}
	}

	// Save off transient overlap samples for clean mix/transition to next transient
	if(flushedOutput.GetSize() >= transientSectionOverlapSampleCount_)
	{
		transientSectionOverlap_.Append(flushedOutput.Retrieve(transientSectionOverlapSampleCount_));
	}

	return audioToReturn;
}

AudioData PhaseVocoderMediator::ProcessAudioWithResampler(const AudioData& audioInputData)
{
	resampler_->SubmitAudioData(audioInputData);

	AudioData dataToReturn;

	while(resampler_->OutputSamplesAvailable())
	{
		std::size_t samplesToRetrieve{bufferSize_};
		if(samplesToRetrieve > resampler_->OutputSamplesAvailable())
		{
			samplesToRetrieve = resampler_->OutputSamplesAvailable();
		}

		dataToReturn.Append(resampler_->GetAudioData(samplesToRetrieve));
	}

	return dataToReturn;
}

void PhaseVocoderMediator::InstantiateWaveFileObjects()
{
	if(!settings_.InputWaveFileGiven())
	{
		Utilities::ThrowException("No input wave file given to PhaseVocoderMediator");
	}

	waveReader_.reset(new WaveFile::WaveFileReader{settings_.GetInputWaveFile()});

	if(settings_.OutputWaveFileGiven())
	{
		std::size_t outputSampleRate{waveReader_->GetSampleRate()};
		if(settings_.ResampleValueGiven())
		{
			outputSampleRate = settings_.GetResampleValue();
		}
	
		waveWriter_.reset(new WaveFile::WaveFileWriter(settings_.GetOutputWaveFile(), 
																static_cast<uint16_t>(waveReader_->GetChannels()), 
																static_cast<uint32_t>(outputSampleRate), 
																static_cast<uint16_t>(waveReader_->GetBitsPerSample())));
	}
}

void PhaseVocoderMediator::InstantiatePhaseVocoder(std::size_t sampleLengthOfAudioToProcess)
{
	if(!settings_.GetStretchFactor() && !settings_.PitchShiftValueGiven())
	{
		// No Phase Vocoder needed
		return;
	}

	double stretchFactor{1.0};
	if(settings_.StretchFactorGiven())
	{
		stretchFactor = settings_.GetStretchFactor();	
	}

	if(settings_.PitchShiftValueGiven())
	{
		stretchFactor *= GetPitchShiftRatio();
	}

	phaseVocoder_.reset(new Signal::PhaseVocoder(waveReader_->GetSampleRate(), sampleLengthOfAudioToProcess, stretchFactor));
}

void PhaseVocoderMediator::InstantiateResampler()
{
	if(!settings_.ResampleValueGiven() && !settings_.PitchShiftValueGiven())
	{
		// No Resampler needed
		return;
	}

	resampler_.reset(new Signal::Resampler(waveReader_->GetSampleRate(), GetResampleRatio()));
}

double PhaseVocoderMediator::GetPitchShiftRatio()
{
	// Google tells me the ratio of a semitone change in pitch can be found by 2^(semitone/12)
	return pow(2.0, settings_.GetPitchShiftValue() / 12.0);
}

double PhaseVocoderMediator::GetResampleRatio()
{
	double resampleRatio{1.0};

	if(settings_.ResampleValueGiven())
	{
		resampleRatio = static_cast<double>(settings_.GetResampleValue()) / static_cast<double>(waveReader_->GetSampleRate());
	}

	if(settings_.PitchShiftValueGiven())
	{
		resampleRatio = resampleRatio / GetPitchShiftRatio();
	}

	return resampleRatio;
}
