/*
 * AudioLib
 *
 * Copyright (c) 2017 - Terence M. Darwen - tmdarwen.com
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <map>
#include <algorithm>
#include <Signal/PhaseVocoder.h>
#include <Signal/Fourier.h>
#include <Signal/PeakFrequencyDetection.h>
#include <Signal/Windowing.h>
#include <Signal/PeakProfile.h>
#include <Utilities/Exception.h>
#include <iostream>

Signal::PhaseVocoder::PhaseVocoder(std::size_t sampleRate, std::size_t inputLength, double stretchFactor) :
	sampleRate_{sampleRate}, 
	inputLength_{inputLength},
	stretchFactor_{stretchFactor},
	minimumOutputSamplesNecessary_{static_cast<std::size_t>(static_cast<double>(inputLength_) * stretchFactor_ + 0.5)}
{
	if(!CheckForEdgeCases()) 
	{
		// If there are no edge cases detected, we'll carry on w/ typical stretching, so calculations are necessary
		DoPrecalculations();
	}
}

Signal::PhaseVocoder::~PhaseVocoder()
{

}

void Signal::PhaseVocoder::SubmitAudioData(const AudioData& audioData)
{
	std::lock_guard<std::mutex> guard(mutex_);

	if(noStretch_)  // Check for edge case
	{
		HandleNoStretchInput(audioData);
		return;
	}

	inputData_.Append(audioData);

	if(shortInputCompress_)  // Check for edge case
	{
		return;
	}

	// Do normal processing.  The first condition in the while statement is necessary since we need at least FFT_SIZE input samples 
	// to process output.  The second condition is necessary since it's possible to have a sample advancement value of zero, 
	// which, without this condition, would leave us stuck in this loop.
	while(inputData_.GetSize() >= FFT_SIZE && (totalOutputSamplesCreated_ < minimumOutputSamplesNecessary_))
	{
		ProcessBuffer();
	}
}

AudioData Signal::PhaseVocoder::GetAudioData(uint64_t samples)
{
	std::lock_guard<std::mutex> guard(mutex_);

	uint64_t samplesToRetrieve{samples};
	if(outputData_.GetSize() < samplesToRetrieve)
	{
		samplesToRetrieve = outputData_.GetSize();
	}

	return outputData_.RetrieveRemove(samplesToRetrieve);
}

AudioData Signal::PhaseVocoder::FlushAudioData()
{
	std::lock_guard<std::mutex> guard(mutex_);

	if(shortInputCompress_)  // Check for edge case
	{
		return HandleShortInputCompress();
	}

	std::size_t windowsToFlush{windowsInUse_.size()};
	std::size_t outputSamplesLimit{(minimumOutputSamplesNecessary_ + (windowsToFlush * QUARTER_FFT_SIZE))};

	// The following is normal processing.  We want to flush the buffer by adding silence to whatever remains in the 
	// input data and process it.
	do
	{
		inputData_.AddSilence(FFT_SIZE - inputData_.GetSize());
		ProcessBuffer();
	} while(windowsProcessed_ <= OVERLAP_FACTOR || totalOutputSamplesCreated_ < outputSamplesLimit);

	AudioData audioData{outputData_};
	outputData_.Clear();

	return audioData;
}

// Returns the stretch factor given at construction
double Signal::PhaseVocoder::GetStretchFactor()
{
	return stretchFactor_;
}

AudioData Signal::PhaseVocoder::HandleShortInputCompress()
{
	auto outputSampleCount{static_cast<std::size_t>(static_cast<double>(inputData_.GetSize()) * stretchFactor_ + 0.5)};
	AudioData audioData(inputData_);
	audioData.Truncate(outputSampleCount);
	inputData_.Clear();
	return audioData;
}

std::size_t Signal::PhaseVocoder::OutputSamplesAvailable()
{
	std::lock_guard<std::mutex> guard(mutex_);

	return outputData_.GetSize();
}

void Signal::PhaseVocoder::Reset()
{
	std::lock_guard<std::mutex> guard(mutex_);

	inputData_.Clear();
	transientSamples_.Clear();
	windowsInUse_.clear();
	outputData_.Clear();
	previousWrappedPhases_.clear();
	previousExtrapolatedUnwrappedPhases_.clear();
	windowsProcessed_ = 0;
	totalOutputSamplesCreated_ = 0;
	inputSamplesProcessed_ = 0;
	sampleAdvancementRemainder_ = 0.0;
}

bool Signal::PhaseVocoder::CheckForEdgeCases()
{
	if(CheckForNoStretchEdgeCase())
	{
		return true;
	}

	if(CheckForShortInputEdgeCases())
	{
		return true;
	}

	return false;
}

bool Signal::PhaseVocoder::CheckForNoStretchEdgeCase()
{
	if(stretchFactor_ == 1.0)
	{
		noStretch_ = true;	
	}

	return false;
}

bool Signal::PhaseVocoder::CheckForShortInputEdgeCases()
{
	if(inputLength_ < FFT_SIZE && stretchFactor_ > 1.0)
	{
		sampleAdvancement_ = 0.0;
		transientCutoff_ = 0.0;
		return true;
	}

	double totalOutputLengthNeeded{static_cast<double>(inputLength_) * stretchFactor_};
	if(totalOutputLengthNeeded < FFT_SIZE)
	{
		shortInputCompress_ = true;
		return true;
	}

	return false;
}

// The following calculates the transientCutoff_ and sampleAdvancement_
void Signal::PhaseVocoder::DoPrecalculations()
{
	double fftSizeOneEighth{QUARTER_FFT_SIZE / 2};

	double totalOutputLengthNeeded{static_cast<double>(inputLength_) * stretchFactor_};
	double totalstretchLengthNeeded{totalOutputLengthNeeded - optimalTransientCutoff_};
	double remainder1024{fmod(totalOutputLengthNeeded, static_cast<double>(QUARTER_FFT_SIZE))};

	double shortestDistanceToResolveRemainder{0.0};
	if(remainder1024 > fftSizeOneEighth)
	{
	   shortestDistanceToResolveRemainder = static_cast<double>(QUARTER_FFT_SIZE) - remainder1024;
	}
	else
	{
	   shortestDistanceToResolveRemainder = -1.0 * remainder1024;
	}
	
	transientCutoff_ = optimalTransientCutoff_ - shortestDistanceToResolveRemainder;
	double adjustedTotalStretchLengthNeeded{totalstretchLengthNeeded + shortestDistanceToResolveRemainder};
	
	double summationSteps{adjustedTotalStretchLengthNeeded / static_cast<double>(QUARTER_FFT_SIZE)};
	double totalWindowsNeeded{summationSteps + (OVERLAP_FACTOR - 1)};
	
	sampleAdvancement_ =  (static_cast<double>(inputLength_) - static_cast<double>(FFT_SIZE)) / (totalWindowsNeeded - 1.0);
}

// This helps handle the simple case where there is a stretch factor of 1.0 (i.e. "no stretch").  In this case we 
// simply copy the input to the output buffer.
void Signal::PhaseVocoder::HandleNoStretchInput(const AudioData& audioData)
{
	outputData_.Append(audioData);
	totalOutputSamplesCreated_ += audioData.GetSize();
}

void Signal::PhaseVocoder::ProcessBuffer()
{
	if(inputData_.GetSize() < FFT_SIZE)
	{
		// We need at least a single FFT_SIZE worth of data to process one window worth of output
		return;
	}

	// The "sample advancement" is the key to how stretching works.  Processing one window always results in 1024 samples.  
	// The stretching and compressing is all performed by how much (or little) we advance the input between windows.
	std::size_t advancement{static_cast<std::size_t>(sampleAdvancement_ + sampleAdvancementRemainder_ + 0.5)};

	// Here we get the next input window, apply the Blackman window, and do the Fourier transform to get the phases.
	auto inputWindow{inputData_.Retrieve(FFT_SIZE)};
	Signal::BlackmanWindow(inputWindow.GetDataWriteAccess());
	auto frequencyDomain{Signal::Fourier::ApplyFFT(inputWindow)};

	// Next we do the actual processing
	if(windowsProcessed_ == 0)
	{
		HandleFirstWindow(frequencyDomain);
	}
	else
	{
		CreateSynthesizedOutputWindow(frequencyDomain, advancement);
	}

	// And finally we do the advancement of the buffer, sample counts, etc
	inputData_.RemoveFrontSamples(advancement);
	sampleAdvancementRemainder_ = sampleAdvancementRemainder_ + (sampleAdvancement_ - advancement);

	inputSamplesProcessed_ += advancement;

	++windowsProcessed_;
}

// The first window does no stretching, since, well, it's the first window.  It also obtains the transient audio.
void Signal::PhaseVocoder::HandleFirstWindow(Signal::FrequencyDomain& frequencyDomain)
{
	auto wrappedPhases = frequencyDomain.GetWrappedPhases();

	// Calculate how many samples we need to retrieve for the transient
	std::size_t samplesToRetrieve{static_cast<std::size_t>(transientCutoff_ + 0.5) + QUARTER_FFT_SIZE};
	if(samplesToRetrieve > inputData_.GetSize())
	{
		samplesToRetrieve = inputData_.GetSize();
	}

	// Save off the transient audio
	transientSamples_ = inputData_.Retrieve(samplesToRetrieve);

	// Pass the first buffer to the output stage unaltered since there is no stretching on the first buffer 
	auto audioData{inputData_.Retrieve(static_cast<uint64_t>(FFT_SIZE))};
	OverlapAndAddForOutput(audioData);

	// Save off our starting phases as our starting point
	previousWrappedPhases_ = wrappedPhases;
	previousExtrapolatedUnwrappedPhases_ = wrappedPhases;
}

void Signal::PhaseVocoder::CreateSynthesizedOutputWindow(Signal::FrequencyDomain& frequencyDomain, std::size_t advancement)
{
	auto wrappedPhases = frequencyDomain.GetWrappedPhases();

	// The PeakProfile will find all the "peaks" in the frequency domain.  We will then use it to find out what the 
	// local peak bin is for a given frequency bin.
	Signal::PeakProfile peakProfile{frequencyDomain};

	// The timeDomainSignal is the unaltered input signal.  This is needed when determining the peak frequency bins 
	// below.  If we give this process the time domain signal with a Blackman window already applied to it the peak 
	// frequency calculations will be wrong.
	auto timeDomainSignal{inputData_.Retrieve(FFT_SIZE)};

	// Get the frequency for each peak bin from the PeakProfile.  We could wait and do it in the for loop below, but if we 
	// did we would be re-calculating the same peak frequency for every bin that had that bin as a local peak.
	auto frequencyPeaks{GetPeakFrequencies(timeDomainSignal, peakProfile)};
	
	// This will store the new frequency domain after we're done processing this window
	Signal::FrequencyDomain newFrequencyDomain;

	// I'm calculating the magnitudes up here so we don't recalculate them for each bin
	auto magnitudes = frequencyDomain.GetMagnitudes();

	// In this loop we calculate the new phase for each frequency bin, which then allows us to calculate new ReX and ImX values.
	for(std::size_t currentBin = 0; currentBin < wrappedPhases.size(); ++currentBin)
	{
		std::size_t localPeakBin{peakProfile.GetLocalPeakForBin(currentBin)};
		double peakFrequency{frequencyPeaks[localPeakBin]};

		// The heart of the whole Phase Vocoder is what happens in this called method
		auto newPhaseWrapped{CalculateNewPhaseWrapped(currentBin, wrappedPhases[currentBin], peakFrequency, advancement)};

		// Now that we have the new phase values, we caluclate the new (synthesized) frequency bins
		double cosPhase{cos(newPhaseWrapped)};
		double sinPhase{sin(newPhaseWrapped)};
		Signal::FrequencyBin newFrequencyBin{magnitudes[currentBin] * cosPhase, magnitudes[currentBin] * sinPhase};
		newFrequencyDomain.PushFrequencyBin(newFrequencyBin);

		// Save off the phases for the next window
		previousExtrapolatedUnwrappedPhases_[currentBin] = newPhaseWrapped;
		previousWrappedPhases_[currentBin] = wrappedPhases[currentBin];
	}

	// Now that we have the new frequency domain signal, we can apply a inverse Fourier transform to get it back to the time domain...
	auto synthisizedSignal{Signal::Fourier::ApplyInverseFFT(newFrequencyDomain)};


	// Then hand it to the overlap-and-add procedure
	OverlapAndAddForOutput(synthisizedSignal);
}

// Here we calculate the frequency for each peak bin from the PeakProfile and return it in a std::map where the key is the peak 
// bin index and the value is the peak frequency value in Hz for the peak bin.  We could wait and do it in the for loop below.
// did we would be re-calculating the same peak frequency for every bin that had that bin as a local peak.
std::map<std::size_t, double> Signal::PhaseVocoder::GetPeakFrequencies(const AudioData& timeDomainSignal, Signal::PeakProfile& peakProfile)
{
	// The GetPeakFrequencyByQuinn() call needs the FFT of the time domain signal to do it's calculation.  If you look at it's 
	// function prototypes you'll see two methods: One where it takes the time domain signal and another where it takes the 
	// real and imaginary frequency components.  By giving it the real and imaginary signals we can just do the FFT outside 
	// the loop and save the time of doing the same FFT over and over.
	auto frequencyDomain{Signal::Fourier::ApplyFFT(AudioData(timeDomainSignal))};

	std::map<std::size_t, double> frequencyPeaks;
	auto getPeakFrequency{[&](std::size_t peakBin)
	{
		frequencyPeaks[peakBin] = Signal::GetPeakFrequencyByQuinn(static_cast<int32_t>(peakBin), timeDomainSignal.GetSize(), frequencyDomain.GetRealComponent(), frequencyDomain.GetImaginaryComponent(), static_cast<double>(sampleRate_));
	}};
	std::for_each(peakProfile.GetAllPeakBins().begin(), peakProfile.GetAllPeakBins().end(), getPeakFrequency);

	return frequencyPeaks;
}

// This method is at the heart of what makes the Phase Vocoder work
double Signal::PhaseVocoder::CalculateNewPhaseWrapped(std::size_t currentBin, double currentWrappedPhase, double peakFrequency, std::size_t advancement)
{
	if(advancement == 0)
	{
		return currentWrappedPhase;
	}

	double advancementAsFloat{static_cast<double>(advancement)};

	// Calculate how far we expect the cycle to advance over "advancement" samples
	double peakFrequencyCycleAdvance = (advancementAsFloat / static_cast<double>(sampleRate_)) * peakFrequency;

	double peakBinExpectedPhaseChange = peakFrequencyCycleAdvance * TWO_PI_RADIANS;
	peakBinExpectedPhaseChange = ConvertUnwrappedPhaseToWrappedPhase(peakBinExpectedPhaseChange);

	double differenceBetweenCalculatedPhases = 0.0;
	if(currentWrappedPhase > previousWrappedPhases_[currentBin])
	{
		differenceBetweenCalculatedPhases = currentWrappedPhase - previousWrappedPhases_[currentBin];
	}
	else
	{
		differenceBetweenCalculatedPhases = currentWrappedPhase + (TWO_PI_RADIANS - previousWrappedPhases_[currentBin]);
	}

	double delta = differenceBetweenCalculatedPhases - peakBinExpectedPhaseChange;
	double deltaWrapped = ConvertUnwrappedPhaseToWrappedPhase(delta);

	double hzPerFourierBin{static_cast<double>(sampleRate_)/static_cast<double>(FFT_SIZE)};
	double expectedPhaseAdvancementForBinPerSample = (peakFrequency/hzPerFourierBin) * (TWO_PI_RADIANS / static_cast<double>(FFT_SIZE));
	double deltaPerSample = deltaWrapped / advancementAsFloat;
	double tweakedPhaseAdvancementForBinPerSample = expectedPhaseAdvancementForBinPerSample + deltaPerSample;
	double extrapolatedPhaseAdvancement = static_cast<double>(QUARTER_FFT_SIZE) * tweakedPhaseAdvancementForBinPerSample;
	double newPhaseUnwrapped = previousExtrapolatedUnwrappedPhases_[currentBin] + extrapolatedPhaseAdvancement;

	double newPhaseWrapped = ConvertUnwrappedPhaseToWrappedPhase(newPhaseUnwrapped);

	return newPhaseWrapped;
}

// This function handles the overlap-and-add process for the synthesized windows
void Signal::PhaseVocoder::OverlapAndAddForOutput(AudioData& newSythesizedWindow)
{
	// Get rid of the oldest of the four past windows since we'll no longer need it now that 
	// we're adding a new window.
	if(windowsInUse_.size() == 4)
	{
		windowsInUse_.pop_front();	
	}

	// Prep the new window and add it into the list of past windows
	BlackmanWindow(newSythesizedWindow.GetDataWriteAccess());
	newSythesizedWindow.Amplify(SYNTHEIZED_OVERLAP_AMP_FACTOR);
	windowsInUse_.push_back(newSythesizedWindow);

	// Now perform the overlap and add with the previous synthesized windows *if* we have enough windows
	std::vector<double> accumulatedSamples(QUARTER_FFT_SIZE, 0.0);
	if(windowsInUse_.size () == 4)
	{
		std::size_t windowCount = windowsInUse_.size();
		for(auto window : windowsInUse_)
		{
			--windowCount;
			auto windowData{window.GetData()};
			for(unsigned int i = 0; i < QUARTER_FFT_SIZE; ++i)
			{
				accumulatedSamples[i] += windowData[windowCount * QUARTER_FFT_SIZE + i];
			}
		}
	}

	if(transientSamples_.GetSize())
	{
		if(transientSamples_.GetSize() > QUARTER_FFT_SIZE)
		{
			std::size_t outputSamples{transientSamples_.GetSize() - QUARTER_FFT_SIZE};
			outputData_ = transientSamples_.RetrieveRemove(outputSamples);
			totalOutputSamplesCreated_ += outputSamples;
		}
		else if(transientSamples_.GetSize() == QUARTER_FFT_SIZE && windowsInUse_.size() == 4)
		{
			auto resultingAudio{MixAtBestCorrelation(transientSamples_.RetrieveRemove(QUARTER_FFT_SIZE), AudioData(accumulatedSamples))};
			outputData_.Append(resultingAudio);
			totalOutputSamplesCreated_ += resultingAudio.GetSize();
		}
	}
	else if(windowsInUse_.size() == 4)
	{
		// And we finally have a new output buffer of 1024 samples so we add that to our FIFO output data
		outputData_.PushBuffer(accumulatedSamples);
		totalOutputSamplesCreated_ += accumulatedSamples.size();
	}
}

double Signal::PhaseVocoder::ConvertUnwrappedPhaseToWrappedPhase(double unwrappedPhase)
{
	return fmod(unwrappedPhase, TWO_PI_RADIANS);
}

// This is a method that performs mixing of two audio signals but does so to avoid phase cancellation.
AudioData Signal::PhaseVocoder::MixAtBestCorrelation(const AudioData& transientBuffer, const AudioData& stretchBuffer)
{
	// We first do some up from sanity checks

	if(transientBuffer.GetSize() != stretchBuffer.GetSize())
	{
		Utilities::ThrowException("PhaseVocoder: TransientBuffer and StretchBuffer differ in size");
	}

	if(transientBuffer.GetSize() != QUARTER_FFT_SIZE)
	{
		Utilities::ThrowException("PhaseVocoder: TransientBuffer differs from expected size");
	}

	if(stretchBuffer.GetSize() != QUARTER_FFT_SIZE)
	{
		Utilities::ThrowException("PhaseVocoder: StretchBuffer differs from expected size");
	}


	// We then do correlation on the first 256 samples to see what matches best

	auto transientData{transientBuffer.GetData()};
	auto stretchData{stretchBuffer.GetData()};

	struct
	{
		std::size_t sampleIndex_;
		double correlationValue_;
	} bestCorrelationFit;

	for(int i = 0; i < QUARTER_FFT_SIZE/4; ++i)
	{
		double correlationValue{0};
		for(int j = 0; j < QUARTER_FFT_SIZE/2; ++j)
		{
			correlationValue += (transientData[j] * stretchData[i + j]);
		}

		if(i == 0 || bestCorrelationFit.correlationValue_ < correlationValue)
		{
			bestCorrelationFit.sampleIndex_ = i;
			bestCorrelationFit.correlationValue_ = correlationValue;
		}
	}

	AudioData transientBufferModified{transientBuffer};
	AudioData stretchBufferModified{stretchBuffer};

	if(bestCorrelationFit.sampleIndex_ > 0)
	{
		// Remove the front bestCorrelationFit.sampleIndex_ samples from the transient buffer
		transientBufferModified.Truncate(transientBufferModified.GetSize() - bestCorrelationFit.sampleIndex_);

		// Remove the last bestCorrelationFit.sampleIndex_ samples from the stretch buffer
		stretchBufferModified.RemoveFrontSamples(bestCorrelationFit.sampleIndex_);
	}

	// Here we do a sanity check that both buffer have the same number of samples
	if(transientBufferModified.GetSize() != stretchBufferModified.GetSize())
	{
		Utilities::ThrowException("PhaseVocoder: Modified TransientBuffer and Modified StretchBuffer differ in size");
	}

	// Now crossfade the two buffers
	transientBufferModified.LinearCrossfade(stretchBufferModified);

	return transientBufferModified;
}