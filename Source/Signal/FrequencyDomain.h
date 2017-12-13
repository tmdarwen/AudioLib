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

//! @file FrequencyDomain.h
//! @brief Class to hold frequency domain information for a signal.

#pragma once

#include <AudioData/AudioData.h>
#include <vector>

namespace Signal  {

//! Simple structure to hold a single frequency bin's frequency domain data.

struct FrequencyBin
{
	//! Instantiates frequency bin data with zeroes for real and imaginary values.
	FrequencyBin() : reX_(0.0), imX_(0.0) { }

	//! Instantiates frequency bin data with the given values.
	FrequencyBin(double reX, double imX) : reX_(reX), imX_(imX) { }

	//! The real component of the frequency.
	double reX_;

	//! The imaginary component of the frequency.
	double imX_;
};


//! Class to hold frequency domain information for a signal.

class FrequencyDomain
{
	public:

		//! Constructs object with no frequency data.
		FrequencyDomain();

		//! Constructs object with the given frequency domain data.
		FrequencyDomain(std::vector<Signal::FrequencyBin> FrequencyBin);

		//! Add the given frequency bin data.
		void PushFrequencyBin(Signal::FrequencyBin FrequencyBin);

		//! Get the number of frequency bins in this frequency domain data.
		std::size_t GetSize() const;

		//! Get frequency bin data for the given frequency bin.
		const FrequencyBin& GetBin(std::size_t binNumber) const;

		//! Get the magnitudes for all frequency bins.
		const std::vector<double>& GetMagnitudes();

		//! Get the wrapped phase values for all frequency bins.
		const std::vector<double>& GetWrappedPhases();

		//! Get the real components of all frequency bins.
		const std::vector<double>& GetRealComponent();

		//! Get the imaginary components of all frequency bins.
		const std::vector<double>& GetImaginaryComponent();

		//! Get the data for all frequency bins.
		std::vector<Signal::FrequencyBin> GetRectangularFrequencyData() const;

	private:
		double CalculateArcTangent(double imaginary, double real);
		enum Quadrant
		{
		        QUADRANT1,
		        QUADRANT2,
		        QUADRANT3,
		        QUADRANT4,
		        BETWEEN_QUADRANT1_AND_QUADRANT2,
		        BETWEEN_QUADRANT2_AND_QUADRANT3,
		        BETWEEN_QUADRANT3_AND_QUADRANT4,
		        BETWEEN_QUADRANT4_AND_QUADRANT1
		};

		Quadrant GetQuadrant(double reX, double imX);
		double GetWrappedPhase(double reX, double imX);

		std::vector<FrequencyBin> data_;

		// Cached data, lazy initialized
		std::vector<double> magnitudes_;
		std::vector<double> wrappedPhases_;
		std::vector<double> realComponent_;
		std::vector<double> imaginaryComponent_;
};

}
