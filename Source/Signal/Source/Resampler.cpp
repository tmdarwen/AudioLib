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

#include <Signal/Resampler.h>
#include <Utilities/Stringify.h>
#include <Utilities/Exception.h>
#include <Signal/Source/WindowedSincValues.h>
#include <Signal/LowPassFilter.h>

// To understand how this works in detail please see the document ResamplingUsingWindowedSincFilter.odg in Sabbatical Notes

Signal::Resampler::Resampler(std::size_t inputSampleRate, double resampleRatio) :
	inputSampleRate_{inputSampleRate}, 
	resampleRatio_{resampleRatio}
{
	ValidateSampleRates();
	InstantiateLowPassFilter();	
	CalculateXSincCenterAdjustmentPerInputSample();
	inputData_.AddSilence(samplesPerSide_); // See the document ResamplingUsingWindowedSincFilter.odg in SabbaticalNotes for why we do this.
}

Signal::Resampler::~Resampler()
{

}

void Signal::Resampler::Reset()
{
	inputData_.Clear();
	outputData_.Clear();
	currentXSincPosition_ = 0.0;
	inputSampleIndex_ = samplesPerSide_;
	inputData_.AddSilence(samplesPerSide_); // See the document ResamplingUsingWindowedSincFilter.odg in SabbaticalNotes for why we do this.
}

void Signal::Resampler::SubmitAudioData(const AudioData& audioData)
{
	std::lock_guard<std::mutex> guard(mutex_);

	if(resampleRatio_ == 1.0)  // Check for edge case
	{
		HandleNoSampleRateChange(audioData);
		return;
	}

	Process(audioData);
}

AudioData Signal::Resampler::GetAudioData(uint64_t samples)
{
	std::lock_guard<std::mutex> guard(mutex_);

	uint64_t samplesToRetrieve{samples};
	if(outputData_.GetSize() < samplesToRetrieve)
	{
		samplesToRetrieve = outputData_.GetSize();
	}

	return outputData_.RetrieveRemove(samplesToRetrieve);
}

std::size_t Signal::Resampler::OutputSamplesAvailable()
{
	std::lock_guard<std::mutex> guard(mutex_);
	return outputData_.GetSize();
}

AudioData Signal::Resampler::FlushAudioData()
{
	std::lock_guard<std::mutex> guard(mutex_);

	// First get any output audio data remaining in the outputData_ buffer
	AudioData audioDataToReturn{outputData_.RetrieveRemove(outputData_.GetSize())};

	// Then process any input samples that might remain
	if(inputData_.GetSize() > 0)
	{
		// Process whatever samples remain by adding silence to the right side.  See the document ResamplingUsingWindowedSincFilter.odg 
		// in SabbaticalNotes for details.  Also, it might help to look at it like we're adding "right side" samples of silence so 
		// we can flush the given input.
		AudioData silence;
		silence.AddSilence(samplesPerSide_ + 1);
		Process(silence);
		audioDataToReturn.Append(outputData_);
		inputData_.Clear();
		outputData_.Clear();
	}

	return audioDataToReturn;
}

void Signal::Resampler::ValidateSampleRates()
{
	if(inputSampleRate_ < minimumSampleRate_ || inputSampleRate_ > maximumSampleRate_)
	{
		Utilities::ThrowException(Utilities::CreateString(" ", "Input sample rate of ", inputSampleRate_, 
																" out of range.  Min:", minimumSampleRate_, "Max:", maximumSampleRate_));
	}

	double outputSampleRate{static_cast<double>(inputSampleRate_) * resampleRatio_};

	if(outputSampleRate < minimumSampleRate_ || outputSampleRate > maximumSampleRate_)
	{
		Utilities::ThrowException(Utilities::CreateString(" ", "Resample ratio results in an output sample rate of ", outputSampleRate, 
																" this is out of range.  Sample rate min:", minimumSampleRate_, "Max:", maximumSampleRate_));
	}
}

void Signal::Resampler::InstantiateLowPassFilter()
{
	if(resampleRatio_ > 1.0)
	{
		// No need for low pass filter if output sample rate is >= input sample rate
		return;
	}

	// The multiplication of 0.5 might at first seem confusing here.  Recall Nyquist. The max frequency in the signal can be one 
	// half of the sample rate of the audio.  The output from the resampler cannot contain audio less than half of the new sample 
	// rate.
	double lowPassRatio{resampleRatio_ * 0.5};
	lowPassFilter_.reset(new Signal::LowPassFilter(lowPassRatio));
}

void Signal::Resampler::CalculateXSincCenterAdjustmentPerInputSample()
{
	xSincCenterAdjustmentPerInputSample_ = Signal::SINC_SAMPLES_PER_X_INTEGER - (SINC_SAMPLES_PER_X_INTEGER / resampleRatio_);
}

// This helps handle the simple case where there is no change between the input sample rate and the output 
// sample rate.  In this case we simply copy the input to the output buffer.
void Signal::Resampler::HandleNoSampleRateChange(const AudioData& audioData)
{
	outputData_.Append(audioData);
}

// This is where the actual resampling occurs - processing input samples through the windowed sinc filter
void Signal::Resampler::Process(const AudioData& audioData)
{
	// Apply a low pass filter to the input if the output sample rate is less than the input sample rate
	if(resampleRatio_ < 1.0)
	{
		inputData_.Append(LowPassFilterInput(audioData));
	}
	else
	{
		inputData_.Append(audioData);	
	}

	// No processing to do if we don't have the minimum requires samples for processing
	if(inputData_.GetSize() < minimumSamplesNeededForProcessing_)
	{
		return;
	}

	auto inputBuffer{inputData_.GetData()};

	while(inputSampleIndex_ < (inputBuffer.size() - samplesPerSide_))
	{
		double outputSample = inputBuffer[inputSampleIndex_] * Signal::GetSincValue(currentXSincPosition_);
		double leftXSincPosition{currentXSincPosition_ - Signal::SINC_SAMPLES_PER_X_INTEGER};
		double rightXSincPosition{currentXSincPosition_ + Signal::SINC_SAMPLES_PER_X_INTEGER};

		for(std::size_t j{1}; j <= samplesPerSide_; ++j)
		{
			// Calculate and add in values for the left and right side of the sinc filter
			outputSample += (inputBuffer[inputSampleIndex_ - j] * Signal::GetSincValue(leftXSincPosition)) +
									(inputBuffer[inputSampleIndex_ + j] * Signal::GetSincValue(rightXSincPosition));

			leftXSincPosition -= Signal::SINC_SAMPLES_PER_X_INTEGER;
			rightXSincPosition += Signal::SINC_SAMPLES_PER_X_INTEGER;
		}

		++inputSampleIndex_;

		currentXSincPosition_ += xSincCenterAdjustmentPerInputSample_;
		CheckForSincPositionWrapping();

		outputData_.PushSample(outputSample);
	}

	DiscardInputNoLongerNeeded();
}

AudioData Signal::Resampler::LowPassFilterInput(const AudioData& audioData)
{
	// A low pass filter will exist only if we are downsampling
	if(lowPassFilter_)
	{
		lowPassFilter_->SubmitAudioData(audioData);
		return lowPassFilter_->GetAudioData(lowPassFilter_->OutputSamplesAvailable());
	}
	else
	{
		Utilities::ThrowException("Resampler attempting to low pass filter when no low pass filter exists");
	}

	return AudioData{};  // We do this just to keep compilers from issuing warnings
}

// See the document ResamplingUsingWindowedSincFilter.odg in SabbaticalNotes for why we need this method.
void Signal::Resampler::CheckForSincPositionWrapping()
{
	if(resampleRatio_ > 1.0)
	{
		while(currentXSincPosition_ >= Signal::SINC_SAMPLES_PER_X_INTEGER)
		{
			currentXSincPosition_ -= Signal::SINC_SAMPLES_PER_X_INTEGER;
			--inputSampleIndex_;				
		}
	}
	else
	{
		while(currentXSincPosition_ <= Signal::SINC_SAMPLES_PER_X_INTEGER)
		{
			currentXSincPosition_ += Signal::SINC_SAMPLES_PER_X_INTEGER;
			++inputSampleIndex_;
		}
	}
}

void Signal::Resampler::DiscardInputNoLongerNeeded()
{
	std::size_t samplesToRemove{inputSampleIndex_ - samplesPerSide_};
	inputData_.RemoveFrontSamples(samplesToRemove);
	inputSampleIndex_ -= samplesToRemove;
}
