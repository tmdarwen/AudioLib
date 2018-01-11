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

#include <Signal/TransientPeakAndValley.h>
#include <cstdint>

Signal::TransientPeakAndValley::TransientPeakAndValley(std::size_t startSamplePosition, std::size_t stepSize) :
	startSamplePoisiton_{startSamplePosition},
	stepSize_{stepSize},
	peakSample_{0},
	valleySample_{0}
{

}

void Signal::TransientPeakAndValley::Reset(std::size_t startSamplePosition, std::size_t stepSize)
{
	startSamplePoisiton_ = startSamplePosition;
	stepSize_ = stepSize;
	peakSample_ = valleySample_ = 0;
	plottedPoints_.clear();
}

std::size_t Signal::TransientPeakAndValley::GetStartSamplePosition() const
{
	return startSamplePoisiton_;
}

std::size_t Signal::TransientPeakAndValley::GetStepSize() const
{
	return stepSize_;
}

std::size_t Signal::TransientPeakAndValley::GetPeakSamplePosition() const
{
	return peakSample_;
}

void Signal::TransientPeakAndValley::SetPeakSamplePosition(std::size_t peak)
{
	peakSample_ = peak;
}

std::size_t Signal::TransientPeakAndValley::GetValleySamplePosition() const
{
	return valleySample_;
}

void Signal::TransientPeakAndValley::SetValleySamplePosition(std::size_t valley)
{
	valleySample_ = valley;
}

std::size_t Signal::TransientPeakAndValley::GetPeakPoint() const
{
	if(stepSize_ == 0)
	{
		return 0;
	}

	return (static_cast<int64_t>(peakSample_) - static_cast<int64_t>(startSamplePoisiton_)) / static_cast<int64_t>(stepSize_);
}

std::size_t Signal::TransientPeakAndValley::GetValleyPoint() const
{
	if(stepSize_ == 0)
	{
		return 0;
	}

	return (static_cast<int64_t>(valleySample_) - static_cast<int64_t>(startSamplePoisiton_)) / static_cast<int64_t>(stepSize_);
}

void Signal::TransientPeakAndValley::PushPlottedPoint(double point)
{
	plottedPoints_.push_back(point);
}

const std::vector<double>& Signal::TransientPeakAndValley::GetPlottedPoints() const
{
	return plottedPoints_;
}
