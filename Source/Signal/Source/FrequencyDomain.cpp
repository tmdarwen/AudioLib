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

#include <Signal/FrequencyDomain.h>
#include <Utilities/Exception.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <iostream>
#include <algorithm>
#include <cassert>

Signal::FrequencyDomain::FrequencyDomain() { }

Signal::FrequencyDomain::FrequencyDomain(std::vector<FrequencyBin> FrequencyBin) : data_{FrequencyBin} { }

void Signal::FrequencyDomain::PushFrequencyBin(FrequencyBin FrequencyBin)
{
	data_.push_back(FrequencyBin);
}

std::size_t Signal::FrequencyDomain::GetSize() const
{
	return data_.size();
}

std::vector<Signal::FrequencyBin> Signal::FrequencyDomain::GetRectangularFrequencyData() const
{
	return data_;
}

const Signal::FrequencyBin& Signal::FrequencyDomain::GetBin(std::size_t binNumber) const
{
	if(binNumber > GetSize())
	{
		Utilities::ThrowException("Attempting to access a frequency bin that does not exist move more samples than exist", GetSize(), binNumber);
	}

	return data_[binNumber];
}

const std::vector<double>& Signal::FrequencyDomain::GetMagnitudes()
{
	if(magnitudes_.size() == 0)
	{
		for(auto frequencyBinValues : data_)
		{
			magnitudes_.push_back(sqrt(frequencyBinValues.reX_ * frequencyBinValues.reX_ + frequencyBinValues.imX_ * frequencyBinValues.imX_));
		}
	}

	return magnitudes_;
}

const std::vector<double>& Signal::FrequencyDomain::GetWrappedPhases()
{
	if(wrappedPhases_.size() == 0)
	{
		for(auto frequencyBinValues : data_)
		{
			wrappedPhases_.push_back(GetWrappedPhase(frequencyBinValues.reX_, frequencyBinValues.imX_));
		}
	}

	return wrappedPhases_;
}

const std::vector<double>& Signal::FrequencyDomain::GetRealComponent()
{
	if(realComponent_.size() == 0)
	{
		for(auto frequencyBinValues : data_)
		{
			realComponent_.push_back(frequencyBinValues.reX_);
		}
	}

	return realComponent_;
}

const std::vector<double>& Signal::FrequencyDomain::GetImaginaryComponent()
{
	if(imaginaryComponent_.size() == 0)
	{
		for(auto frequencyBinValues : data_)
		{
			imaginaryComponent_.push_back(frequencyBinValues.imX_);
		}
	}

	return imaginaryComponent_;
}

Signal::FrequencyDomain::Quadrant Signal::FrequencyDomain::GetQuadrant(double reX, double imX)
{
	if (reX > 0.00)
	{
		if (imX == 0.00)
		{
			return Signal::FrequencyDomain::Quadrant::BETWEEN_QUADRANT4_AND_QUADRANT1;
		}
		else if (imX > 0.00)
		{
			return Signal::FrequencyDomain::Quadrant::QUADRANT1;
		}
		else // imX < 0.00
		{
			return Signal::FrequencyDomain::Quadrant::QUADRANT4;
		}
	}
	else if (reX < 0.00)
	{
		if (imX == 0.00)
		{
			return Signal::FrequencyDomain::Quadrant::BETWEEN_QUADRANT2_AND_QUADRANT3;
		}
		else if (imX > 0.00)
		{
			return Signal::FrequencyDomain::Quadrant::QUADRANT2;
		}
		else // imX < 0.00
		{
			return Signal::FrequencyDomain::Quadrant::QUADRANT3;
		}
	}
	else // reX == 0.00
	{
		if (imX > 0.00)
		{
			return Signal::FrequencyDomain::Quadrant::BETWEEN_QUADRANT1_AND_QUADRANT2;
		}
		else
		{
			return Signal::FrequencyDomain::Quadrant::BETWEEN_QUADRANT3_AND_QUADRANT4;
		}
	}

	static_assert(true, "Failed to find the quadrant");
}

double Signal::FrequencyDomain::GetWrappedPhase(double reX, double imX)
{
	Quadrant quadrant{GetQuadrant(reX, imX)};
	double atanValue{CalculateArcTangent(imX, reX)};

	if (quadrant == QUADRANT1 || quadrant == BETWEEN_QUADRANT1_AND_QUADRANT2 || quadrant == BETWEEN_QUADRANT4_AND_QUADRANT1)
	{
		return atanValue;
	}
	else if (quadrant == QUADRANT2 || quadrant == BETWEEN_QUADRANT2_AND_QUADRANT3)
	{
		return (atanValue + M_PI);
	}
	else if (quadrant == QUADRANT3 || quadrant == BETWEEN_QUADRANT3_AND_QUADRANT4)
	{
		return (atanValue + M_PI);
	}
	else if (quadrant == QUADRANT4)
	{
		return (atanValue + (2.0 * M_PI));
	}

	static_assert(true, "Failed to calculate the phase");

	return 0.0;  // To squelch the warning
}

double Signal::FrequencyDomain::CalculateArcTangent(double imaginary, double real)
{
	if(real == 0.0)
	{
		return 0.0;
	}

	return atan(imaginary / real);
}
