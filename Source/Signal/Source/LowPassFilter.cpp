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

#include <Signal/LowPassFilter.h>
#include <Utilities/Exception.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <algorithm>
#include <numeric>
#include <iostream>

Signal::LowPassFilter::LowPassFilter(double cutoffRatio, std::size_t filterLength) : 
	cutoffRatio_{cutoffRatio},
	filterLength_{filterLength}
{
	std::lock_guard<std::mutex> guard(mutex_);

	if(cutoffRatio_ < minCutoffRatioRange_ || cutoffRatio_ > maxCutoffRatioRange_)
	{
		Utilities::ThrowException("LowPassFilter cutoffRatio is out of range");
	}
	
	CalculateFilterKernel();
}

void Signal::LowPassFilter::Reset()
{
	std::lock_guard<std::mutex> guard(mutex_);

	audioInput_.Clear();
	audioOutput_.Clear();
}

void Signal::LowPassFilter::SubmitAudioData(const AudioData& audioData)
{
	std::lock_guard<std::mutex> guard(mutex_);

	audioInput_.Append(audioData);
	Process();	
}

AudioData Signal::LowPassFilter::GetAudioData(uint64_t samples)
{
	std::lock_guard<std::mutex> guard(mutex_);

	uint64_t samplesToRetrieve{samples};
	if(audioOutput_.GetSize() < samplesToRetrieve)
	{
		samplesToRetrieve = audioOutput_.GetSize();
	}

	return audioOutput_.RetrieveRemove(samplesToRetrieve);
}

std::size_t Signal::LowPassFilter::OutputSamplesAvailable()
{
	std::lock_guard<std::mutex> guard(mutex_);
	return audioOutput_.GetSize();
}

std::size_t Signal::LowPassFilter::MinimumSamplesNeededForProcessing()
{
	return filterLength_;	
}

AudioData Signal::LowPassFilter::FlushAudioData()
{
	std::lock_guard<std::mutex> guard(mutex_);

	audioInput_.AddSilence(filterLength_);
	Process();

	AudioData audioData{audioOutput_};
	audioOutput_.Clear();

	return audioData;
}

void Signal::LowPassFilter::Process()
{
	if(audioInput_.GetSize() < filterLength_)
	{
		return;
	}

	std::size_t samplesToProcess{audioInput_.GetSize() - filterLength_};
	auto inputBuffer{audioInput_.GetData()};

	// Convolve the input signal and filter kernel
	for(uint64_t i = 0; i < samplesToProcess; ++i)
	{
		double accumulator{0};
		for(uint64_t j = 0; j < filterLength_; j++)
		{
			accumulator = accumulator + (inputBuffer[i + j] * filterKernel_[j]);
		}

		if(accumulator > 1.0)
		{
			audioOutput_.PushSample(1.0);
		}
		else if(accumulator < -1.0)
		{
			audioOutput_.PushSample(-1.0);
		}
		else
		{
			audioOutput_.PushSample(accumulator);
		}
	}

	// Remove the samples we just processed
	audioInput_.RemoveFrontSamples(samplesToProcess);
}

void Signal::LowPassFilter::CalculateFilterKernel()
{
	// This is straight out of "The Scientist and Engineer's Guide to Digital Signal Processing" chapter 16 table 16-1

	std::size_t halfFilterLength{filterLength_ / 2};
	double twoPI{2.0 * M_PI};
	for(std::size_t i = 0; i < filterLength_; ++i) {

		if((i - halfFilterLength) / 2 == 0)
		{
			filterKernel_.push_back(twoPI * cutoffRatio_);
		}
		else
		{
			double position{static_cast<double>(i) - halfFilterLength};
			filterKernel_.push_back(sin(twoPI * cutoffRatio_ * position) / position);
		}
			
		// Apply windowing
		filterKernel_[i] = filterKernel_[i] * (0.54 - 0.46 * cos(2 * M_PI * (float)i / filterLength_));
	}
	
	// Normalize the low-pass filter kernal for unity gain at DC
	double filterKernelSum{std::accumulate(filterKernel_.begin(), filterKernel_.end(), 0.0)};
	std::for_each(filterKernel_.begin(), filterKernel_.end(), [&](double& currentIndexValue) { currentIndexValue /= filterKernelSum; });
}
