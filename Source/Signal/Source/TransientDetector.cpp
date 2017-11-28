/*
 * PhaseVocoder
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

#include <Signal/TransientDetector.h>
#include <Signal/Windowing.h>
#include <Signal/Fourier.h>
#include <Signal/PeakFrequencyDetection.h>
#include <Utilities/Exception.h>
#include <Utilities/Stringify.h>

Signal::TransientDetector::TransientDetector(std::size_t sampleRate) : 
	firstLevelStepSize_{static_cast<std::size_t>(static_cast<double>(sampleRate) * (firstLevelStepMilliseconds_ / 1000.0) + 0.5)},
	secondLevelStepSize_{static_cast<std::size_t>(static_cast<double>(sampleRate) * (secondLevelStepMilliseconds_ / 1000.0) + 0.5)},
	thirdLevelStepSize_{static_cast<std::size_t>(static_cast<double>(sampleRate) * (thirdLevelStepMilliseconds_ / 1000.0) + 0.5)},
	samplesOfPastAudioToRetain_{sampleRate * secondsOfPastAudioToRetain_}
{

}

Signal::TransientDetector::~TransientDetector()
{

}

double Signal::TransientDetector::GetValleyToPeakRatio()
{
	return minValleyToPeakGrowthRatio_;
}

void Signal::TransientDetector::SetValleyToPeakRatio(double ratio)
{
	minValleyToPeakGrowthRatio_ = ratio;
}

void Signal::TransientDetector::Reset()
{
	audioDataInput_.Clear();
	transientsFound_ = false;
	lastTransientValue_ = 0;
	inputSamplesProcessed_ = 0;
}

bool Signal::TransientDetector::FindTransients(const AudioData& audioInput, std::vector<std::size_t>& transients)
{
	// First make sure we're not retaining too much past audio
	CheckForOldAudio();

	// Append the newly given audio to our internal buffer
	audioDataInput_.Append(audioInput);

	// Make sure the transient buffer is empty before adding any
	transients.clear();

	// We need a minimum number of samples before any audio can be processed
	if(!CheckForEnoughAudioToProcess())
	{
		return false;
	}

	// Check if the input is silent as there is no sense in looking for peaks in silence
	if(CheckForAllSilence())
	{
		return false;
	}

	// Now we actually find the transients
	return FindTransients(transients);
}

// This makes sure we're not retaining too much past audio
void Signal::TransientDetector::CheckForOldAudio()
{
	if(audioDataInput_.GetSize() > samplesOfPastAudioToRetain_)
	{
		std::size_t samplesToRemove{audioDataInput_.GetSize() - samplesOfPastAudioToRetain_};
		audioDataInput_.RemoveFrontSamples(samplesToRemove);
		inputSamplesProcessed_ += samplesToRemove;
	}
}

// Check for the minimum number of samples needed for audio to be processed.  Returns false 
// if there's not enough available to do any processing.
bool Signal::TransientDetector::CheckForEnoughAudioToProcess()
{
	std::size_t minSamplesNecessaryForProcessing{firstLevelStepSize_ + GetLookAheadSampleCount()};
	if(audioDataInput_.GetSize() < minSamplesNecessaryForProcessing)
	{
		return false;
	}

	return true;
}

// Check if the input is silent as there is no sense in looking for peaks in silence
bool Signal::TransientDetector::CheckForAllSilence()
{
	const std::vector<double>& audioData{audioDataInput_.GetData()};
	std::size_t sampleCount{audioData.size() - GetLookAheadSampleCount()};
	for(std::size_t i{0}; i < sampleCount; ++i)
	{
		if(audioData[i] > 0.0)
		{
			return false;	
		}
	}

	// If we're here, all samples from zero to GetLookAheadSampleCount() are silent

	audioDataInput_.RemoveFrontSamples(sampleCount);
	inputSamplesProcessed_ += sampleCount;
	return true;
}

// Return true if transients are found, false otherwise.
// If transients are found, they will be in the transients variable
bool Signal::TransientDetector::FindTransients(std::vector<std::size_t>& transients)
{
	// The first transient is a bit of a special case.  We handle it here.
	if(transientsFound_ == false)
	{
		auto transientSamplePosition{FindFirstTransient() + inputSamplesProcessed_};
		transients.push_back(transientSamplePosition);
		lastTransientValue_ = transientSamplePosition;
		transientsFound_ = true;
	}

	// Try and find peaks in the audio we have
	Signal::TransientDetector::PeakAndValley firstLevelPeakAndValley;
	while(GetPeakAndValley(audioDataInput_, firstLevelStepSize_, firstLevelPeakAndValley))
	{
		auto transientSamplePosition{inputSamplesProcessed_ + FindTransientSamplePosition(firstLevelPeakAndValley)};
		if(transientsFound_ == false || (3 * firstLevelStepSize_ + lastTransientValue_) <= transientSamplePosition)
		{
			transients.push_back(transientSamplePosition);
			lastTransientValue_ = transientSamplePosition;
			transientsFound_ = true;
		}

		// Remove all audio from the beginning of audioDataInput_ thru the peakIndex samples
		audioDataInput_.RemoveFrontSamples(firstLevelPeakAndValley.peak_ + firstLevelStepSize_);
		inputSamplesProcessed_ += firstLevelPeakAndValley.peak_ + firstLevelStepSize_;
	}

	if(transients.size())
	{
		return true;
	}

	return false;

}

std::size_t Signal::TransientDetector::FindFirstTransient()
{
	std::size_t samplesToCheck{audioDataInput_.GetSize()};
	for(std::size_t i{0}; i < samplesToCheck; ++i)
	{
		if(audioDataInput_.GetData()[i] > 0.0)
		{
			return i;
		}
	}

	Utilities::ThrowException("Failed to find first transient", samplesToCheck);
	return 0;
}

std::size_t Signal::TransientDetector::FindTransientSamplePosition(const Signal::TransientDetector::PeakAndValley& firstLevelPeakAndValley)
{
	Signal::TransientDetector::PeakAndValley secondLevelPeakAndValley;
	std::size_t secondLevelStartPosition{firstLevelPeakAndValley.valley_};
	std::size_t secondLevelLength{(firstLevelPeakAndValley.peak_ - firstLevelPeakAndValley.valley_) + (2 * firstLevelStepSize_)};

	AudioData secondLevelAudioData{audioDataInput_.Retrieve(secondLevelStartPosition, secondLevelLength)};
	GetPeakAndValley(secondLevelAudioData, secondLevelStepSize_, secondLevelPeakAndValley);

	Signal::TransientDetector::PeakAndValley thirdLevelPeakAndValley;
	std::size_t thirdLevelStartPosition{firstLevelPeakAndValley.valley_ + secondLevelPeakAndValley.valley_};
	std::size_t thirdLevelLength{(secondLevelPeakAndValley.peak_ - secondLevelPeakAndValley.valley_) + firstLevelStepSize_};

	AudioData thirdLevelAudioData{audioDataInput_.Retrieve(thirdLevelStartPosition, thirdLevelLength)};
	GetPeakAndValley(thirdLevelAudioData, thirdLevelStepSize_, thirdLevelPeakAndValley);

	return (thirdLevelStartPosition + thirdLevelPeakAndValley.valley_);
}

bool Signal::TransientDetector::SampleIsPeak(double centerSample, double leftSample, double rightSample)
{
	if(centerSample > leftSample && centerSample >= rightSample)
	{
		return true;
	}

	return false;
}

bool Signal::TransientDetector::SampleMeetsPeekRequirements(double peakSampleValue, double valleySampleValue)
{
	double valleyToPeakGrowthRatio{(peakSampleValue - valleySampleValue) / valleySampleValue};
	if(peakSampleValue > minPeakLevel_ && valleyToPeakGrowthRatio >  minValleyToPeakGrowthRatio_)
	{
		return true;
	}

	return false;
}

std::size_t Signal::TransientDetector::GetLookAheadSampleCount()
{
	return 3 * firstLevelStepSize_;
}

double Signal::TransientDetector::GetMaxSample(const AudioData& audioData, std::size_t sampleCount)
{
	const std::vector<double>& sampleBuffer{audioData.GetData()};

	double maxSample{0.0};

	for(std::size_t i{0}; i < sampleCount; ++i)
	{
		double currentSample{sampleBuffer[i]};
		if(currentSample < 0.0)
		{
			currentSample = -1.0 * currentSample;				
		}

		if(maxSample < currentSample)
		{
			maxSample = currentSample;
		}
	}

	return maxSample;
}

bool Signal::TransientDetector::GetPeakAndValley(const AudioData& audioData, std::size_t stepSize, PeakAndValley& peakAndValley)
{
	// To find a peak, we need to analyze at least 3 data points
	if(audioData.GetSize() < (3 * stepSize))
	{
		return false;
	}

	AudioData tempBuffer{audioData};
	std::size_t sampleCounter{0};

	double leftSample{GetMaxSample(tempBuffer, stepSize)};
	tempBuffer.RemoveFrontSamples(stepSize);
	sampleCounter += stepSize;

	double centerSample{GetMaxSample(tempBuffer, stepSize)};
	tempBuffer.RemoveFrontSamples(stepSize);
	sampleCounter += stepSize;

	std::size_t valleySamplePosition{0};
	double valleyValue{leftSample};

	while(tempBuffer.GetSize() >= stepSize)
	{
		auto rightSample{GetMaxSample(tempBuffer, stepSize)};
		if(SampleIsPeak(centerSample, leftSample, rightSample))
		{
			if(SampleMeetsPeekRequirements(centerSample, valleyValue))
			{
				// We subtract stepSize here since we're technically on the "right sample" i.e. one step past the center (peak) sample
				peakAndValley.peak_ = sampleCounter - stepSize;

				if(valleySamplePosition > stepSize)
				{
					peakAndValley.valley_ = valleySamplePosition - stepSize;
				}
				else
				{
					peakAndValley.valley_ = 0;
				}

				return true;
			}

			// If we're here, there was a peak, but it doesn't qualify as an "official" peak, so reset the valley info
			valleyValue = centerSample;
			valleySamplePosition = sampleCounter;
		}
		else if(valleyValue >= centerSample)
		{
			valleyValue = centerSample;
			valleySamplePosition = sampleCounter;
		}

		tempBuffer.RemoveFrontSamples(stepSize);
		sampleCounter += stepSize;

		leftSample = centerSample;
		centerSample = rightSample;
	}

	return false;
}
