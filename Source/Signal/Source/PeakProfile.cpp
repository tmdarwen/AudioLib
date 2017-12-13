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

#include <Signal/PeakProfile.h>
#include <Utilities/Exception.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <iostream>
#include <algorithm>
#include <utility>

Signal::PeakProfile::PeakProfile(const Signal::FrequencyDomain& frequencyDomain)  : frequencyDomain_(frequencyDomain)
{
	CalculatePeaksAndValleys();
}

// This will return the closest peak for the given frequency bin
std::size_t Signal::PeakProfile::GetLocalPeakForBin(std::size_t bin)
{
	if(peakBins_.size() == 0)
	{
		return 0;
	}

	// First find the peak bin
	for(std::size_t i = 0; i < (valleyBins_.size() - 1); ++i)
	{
		if(bin >= valleyBins_[i] && bin < valleyBins_[i + 1])	
		{
			return peakBins_[i];
		}
	}

	return peakBins_[peakBins_.size() - 1]; // If we're here, it must be the final peak
}

const std::vector<std::size_t>& Signal::PeakProfile::GetAllPeakBins()
{
	return peakBins_;
}

std::pair<std::size_t, std::size_t> Signal::PeakProfile::GetValleyBins(std::size_t peakBin)
{
	auto peakPosition = std::find(peakBins_.begin(), peakBins_.end(), peakBin) - peakBins_.begin();
	if(static_cast<std::size_t>(peakPosition) >= peakBins_.size())
	{
		throw Utilities::Exception("Given peak position not found in peak list");
	}

	if(static_cast<std::size_t>(peakPosition + 1) >= valleyBins_.size())
	{
		throw Utilities::Exception("Peak position does not have corresponding valleys");
	}

	return std::make_pair(valleyBins_[peakPosition], valleyBins_[peakPosition + 1]);
}

void Signal::PeakProfile::CalculatePeaksAndValleys()
{
	auto magnitudes{frequencyDomain_.GetMagnitudes()};

	std::vector<double> magnitudesAveraged;

	std::size_t averageSpanLength{10};
	for(std::size_t i{averageSpanLength/2}; i < (magnitudes.size() - averageSpanLength/2); ++i)
	{
		double average{0.0};
		for(std::size_t j{0}; j < averageSpanLength; ++j)
		{
			average += magnitudes[(i + j) - averageSpanLength/2];
		}
		average = average / static_cast<double>(averageSpanLength);

		if(magnitudesAveraged.size() == 0)
		{
			for(std::size_t j{0}; j < averageSpanLength/2; ++j)
			{
				magnitudesAveraged.push_back(average);	
			}
		}

		magnitudesAveraged.push_back(average);	
	}

	double finalAverage{magnitudesAveraged[magnitudesAveraged.size() - 1]};
	for(std::size_t i{magnitudesAveraged.size()}; i < magnitudes.size(); ++i)
	{
		magnitudesAveraged.push_back(finalAverage);	
	}

	std::size_t runningLow{0};

	for(std::size_t i{2}; i < (magnitudes.size() - 2); ++i)
	{ 
		double localAverage{(magnitudes[i - 2] + magnitudes[i - 1] + magnitudes[i] + magnitudes[i + 1] + magnitudes[i + 2]) / 5};

		if(localAverage > magnitudesAveraged[i] && magnitudes[i] > 1.0)  // It has to be above a certain threshold to be a peak (fix this arbitrary "1.0" as the threshold)
		{
			if((magnitudes[i] > magnitudes[i + 1] && magnitudes[i] > magnitudes[i + 2]) &&
			  (magnitudes[i] > magnitudes[i - 1] && magnitudes[i] > magnitudes[i - 2]))
			{
				peakBins_.push_back(i);
				if(peakBins_.size() == 1)
				{
					valleyBins_.push_back(0);
				}
				else
				{
					valleyBins_.push_back(runningLow);
				}

				runningLow = i + 1;
			}
		}

		if(magnitudes[i] < magnitudes[runningLow])	
		{
			runningLow = i;	
		}
	}

	valleyBins_.push_back(magnitudes.size() - 1);
}
