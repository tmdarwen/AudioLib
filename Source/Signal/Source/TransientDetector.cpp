/*
 * AudioLib
 *
 * Copyright (c) 2017-2018 - Terence M. Darwen - tmdarwen.com
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
#include <algorithm>

Signal::TransientDetector::TransientDetector(std::size_t sampleRate) :
	sampleRate_{sampleRate},
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

double Signal::TransientDetector::GetMinimumPeakLevel()
{
	return minPeakLevel_;
}

void Signal::TransientDetector::SetMinimumPeakLevel(double minPeakLevel)
{
	minPeakLevel_ = minPeakLevel;
}

void Signal::TransientDetector::SetStep(double stepInMilliseconds, Signal::TransientDetector::Step step)
{
	switch(step)
	{
		case Step::First:
			firstLevelStepMilliseconds_ = stepInMilliseconds;
			firstLevelStepSize_ = static_cast<std::size_t>(static_cast<double>(sampleRate_) * (stepInMilliseconds / 1000.0) + 0.5);
			break;
		case Step::Second:
			secondLevelStepMilliseconds_ = stepInMilliseconds;
			secondLevelStepSize_ = static_cast<std::size_t>(static_cast<double>(sampleRate_) * (stepInMilliseconds / 1000.0) + 0.5);
			break;
		case Step::Third:
			thirdLevelStepMilliseconds_ = stepInMilliseconds;
			thirdLevelStepSize_ = static_cast<std::size_t>(static_cast<double>(sampleRate_) * (stepInMilliseconds / 1000.0) + 0.5);
			break;
	}
}

void Signal::TransientDetector::SetStepInSamples(std::size_t samples, Signal::TransientDetector::Step step)
{
	switch(step)
	{
		case Step::First:
			firstLevelStepSize_ = samples;
			firstLevelStepMilliseconds_ = static_cast<double>(firstLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
			break;
		case Step::Second:
			secondLevelStepSize_ = samples;
			secondLevelStepMilliseconds_ = static_cast<double>(secondLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
			break;
		case Step::Third:
			thirdLevelStepSize_ = samples;
			thirdLevelStepMilliseconds_ = static_cast<double>(thirdLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
			break;
	}
}


double Signal::TransientDetector::GetStep(Signal::TransientDetector::Step step)
{
	switch(step)
	{
		case Step::First:
			return firstLevelStepMilliseconds_;
		case Step::Second:
			return secondLevelStepMilliseconds_;
		case Step::Third:
			return thirdLevelStepMilliseconds_;
	}

	return 0; // Just to avoid warning
}

std::size_t Signal::TransientDetector::GetStepInSamples(Signal::TransientDetector::Step step)
{
	switch(step)
	{
		case Step::First:
			return firstLevelStepSize_;
		case Step::Second:
			return secondLevelStepSize_;
		case Step::Third:
			return thirdLevelStepSize_;
	}

	return 0; // Just to avoid warning
}

/*
void Signal::TransientDetector::SetFirstLevelStepInSamples(std::size_t samples)
{
	firstLevelStepSize_ = samples;
	firstLevelStepMilliseconds_ = static_cast<double>(firstLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
}

void Signal::TransientDetector::SetSecondLevelStepInSamples(std::size_t samples)
{
	secondLevelStepSize_ = samples;
	secondLevelStepMilliseconds_ = static_cast<double>(secondLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
}

void Signal::TransientDetector::SetThirdLevelStepInSamples(std::size_t samples)
{
	thirdLevelStepSize_ = samples;
	thirdLevelStepMilliseconds_ = static_cast<double>(thirdLevelStepSize_) / static_cast<double>(sampleRate_) * 1000.0;
}
		
double Signal::TransientDetector::GetFirstLevelStep()
{
	return firstLevelStepMilliseconds_;
}

double Signal::TransientDetector::GetSecondLevelStep()
{
	return secondLevelStepMilliseconds_;
}

double Signal::TransientDetector::GetThirdLevelStep()
{
	return thirdLevelStepMilliseconds_;
}
*/

void Signal::TransientDetector::Reset()
{
	audioDataInput_.Clear();
	transientsFound_ = false;
	lastTransientValue_ = 0;
	inputSamplesProcessed_ = 0;
	firstLevelPeakSamplePositions_.clear();
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
	Signal::TransientPeakAndValley firstLevelPeakAndValley(0, firstLevelStepSize_);
	while(GetPeakAndValley(audioDataInput_, firstLevelStepSize_, firstLevelPeakAndValley))
	{
		auto transientSamplePosition{inputSamplesProcessed_ + FindTransientSamplePosition(firstLevelPeakAndValley)};
		if(transientsFound_ == false || (3 * firstLevelStepSize_ + lastTransientValue_) <= transientSamplePosition)
		{
			firstLevelPeakSamplePositions_.push_back(inputSamplesProcessed_ + firstLevelPeakAndValley.GetPeakSamplePosition());
			firstLevel_.push_back(firstLevelPeakAndValley);
			transients.push_back(transientSamplePosition);
			lastTransientValue_ = transientSamplePosition;
			transientsFound_ = true;
		}

		// Remove all audio from the beginning of audioDataInput_ thru the peakIndex samples
		audioDataInput_.RemoveFrontSamples(firstLevelPeakAndValley.GetPeakSamplePosition() + firstLevelStepSize_);
		inputSamplesProcessed_ += firstLevelPeakAndValley.GetPeakSamplePosition() + firstLevelStepSize_;
		firstLevelPeakAndValley.Reset(0, firstLevelStepSize_);
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

std::size_t Signal::TransientDetector::FindTransientSamplePosition(const Signal::TransientPeakAndValley& firstLevelPeakAndValley)
{
	std::size_t secondLevelStartPosition{firstLevelPeakAndValley.GetValleySamplePosition()};
	Signal::TransientPeakAndValley secondLevelPeakAndValley(0, secondLevelStepSize_);
	std::size_t secondLevelLength{(firstLevelPeakAndValley.GetPeakSamplePosition() - firstLevelPeakAndValley.GetValleySamplePosition()) + (2 * firstLevelStepSize_)};

	AudioData secondLevelAudioData{audioDataInput_.Retrieve(secondLevelStartPosition, secondLevelLength)};
	GetPeakAndValley(secondLevelAudioData, secondLevelStepSize_, secondLevelPeakAndValley);
	secondLevel_.push_back(secondLevelPeakAndValley);

	std::size_t thirdLevelStartPosition{firstLevelPeakAndValley.GetValleySamplePosition() + secondLevelPeakAndValley.GetValleySamplePosition()};
	Signal::TransientPeakAndValley thirdLevelPeakAndValley(0, thirdLevelStepSize_);
	std::size_t thirdLevelLength{(secondLevelPeakAndValley.GetPeakSamplePosition() - secondLevelPeakAndValley.GetValleySamplePosition()) + firstLevelStepSize_};
	thirdLevel_.push_back(thirdLevelPeakAndValley);

	AudioData thirdLevelAudioData{audioDataInput_.Retrieve(thirdLevelStartPosition, thirdLevelLength)};
	GetPeakAndValley(thirdLevelAudioData, thirdLevelStepSize_, thirdLevelPeakAndValley);

	return (thirdLevelStartPosition + thirdLevelPeakAndValley.GetValleySamplePosition());
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

std::vector<std::size_t> Signal::TransientDetector::GetFirstLevelPeakSamplePositions()
{
	return firstLevelPeakSamplePositions_;
}

std::vector<double> Signal::TransientDetector::GetFirstStepValues(const AudioData& audioInput)
{
	std::vector<double> firstStepValues;
	for(std::size_t i{0}; i < audioInput.GetSize(); i += firstLevelStepSize_)
	{
		std::size_t samplesToRetrieve{std::min(firstLevelStepSize_, (audioInput.GetSize() - i))};
		auto audio{audioInput.Retrieve(i, samplesToRetrieve)};
		firstStepValues.push_back(GetMaxSample(audio, firstLevelStepSize_));
	}
	return firstStepValues;
}

double Signal::TransientDetector::GetMaxSample(const AudioData& audioData, std::size_t sampleCount)
{
	std::size_t samplesToIterate{std::min(sampleCount, (audioData.GetSize()))};

	const std::vector<double>& sampleBuffer{audioData.GetData()};

	double maxSample{0.0};

	for(std::size_t i{0}; i < samplesToIterate; ++i)
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

bool Signal::TransientDetector::GetPeakAndValley(const AudioData& audioData, std::size_t stepSize, TransientPeakAndValley& peakAndValley)
{
	// To find a peak, we need to analyze at least 3 data points
	if(audioData.GetSize() < (3 * stepSize))
	{
		return false;
	}

	AudioData tempBuffer{audioData};
	std::size_t sampleCounter{0};

	double leftSample{GetMaxSample(tempBuffer, stepSize)};
	peakAndValley.PushPlottedPoint(leftSample);
	tempBuffer.RemoveFrontSamples(stepSize);
	sampleCounter += stepSize;

	double centerSample{GetMaxSample(tempBuffer, stepSize)};
	peakAndValley.PushPlottedPoint(centerSample);
	tempBuffer.RemoveFrontSamples(stepSize);
	sampleCounter += stepSize;

	std::size_t valleySamplePosition{0};
	double valleyValue{leftSample};

	while(tempBuffer.GetSize() >= stepSize)
	{
		auto rightSample{GetMaxSample(tempBuffer, stepSize)};
		peakAndValley.PushPlottedPoint(rightSample);
		if(SampleIsPeak(centerSample, leftSample, rightSample))
		{
			if(SampleMeetsPeekRequirements(centerSample, valleyValue))
			{
				// We subtract stepSize here since we're technically on the "right sample" i.e. one step past the center (peak) sample
				peakAndValley.SetPeakSamplePosition(sampleCounter - stepSize);

				if(valleySamplePosition > stepSize)
				{
					peakAndValley.SetValleySamplePosition(valleySamplePosition - stepSize);
				}
				else
				{
					peakAndValley.SetValleySamplePosition(0);
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
