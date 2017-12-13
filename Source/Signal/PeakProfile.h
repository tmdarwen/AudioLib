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

//! @file PeakProfile.h
//! @brief Identifies peak frequency bins in an audio signal.

#pragma once

#include <AudioData/AudioData.h>
#include <Signal/FrequencyDomain.h>
#include <vector>

namespace Signal  {

//! Given a frequency domain signal, this will find all the "peak" bins.

class PeakProfile
{
	public:
		//! Instantiate the object with frequency domain information.
		PeakProfile(const FrequencyDomain& frequencyDomain);

		//! This will return the closest peak for the given frequency bin.
		std::size_t GetLocalPeakForBin(std::size_t bin);

		//! Returns a list of all peak bins.
		const std::vector<std::size_t>& GetAllPeakBins();

		//! Returns a list of all valley bins.
		std::pair<std::size_t, std::size_t> GetValleyBins(std::size_t peakBin);

	private:
		void CalculatePeaksAndValleys();
		
		// The following hold the bin numbers of peaks (high point of magnitude) and 
		// the low points (or "valleys") on both sides of the peaks.
		std::vector<std::size_t> peakBins_;
		std::vector<std::size_t> valleyBins_;

		// The frequency domain given by the user at construction
		FrequencyDomain frequencyDomain_;	
};

}
