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

//! @file TransientPeakAndValley.h
//! @brief Stores details on transient information.

#pragma once

#include <cstddef>
#include <vector>

namespace Signal {

//! Stores details on transient information.

class TransientPeakAndValley
{
	public:
		//! Sample position of where analysis started and the first level step size used.
		TransientPeakAndValley(std::size_t startSamplePosition, std::size_t stepSize);

		//! Clears the stored data in this structure.
		void Reset(std::size_t startSamplePosition, std::size_t stepSize);

		//! Retrieve the starting position of the transient.
		std::size_t GetStartSamplePosition() const;

		//! Retrieve the step size used to find the transient.
		std::size_t GetStepSize() const;

		//! Retrieve the sample position of the peak obtained during analysis.
		std::size_t GetPeakSamplePosition() const;

		//! Set the sample position of the peak.
		void SetPeakSamplePosition(std::size_t peakPoint);

		//! Retrieve the sample position of the valley obtained during analysis.
		std::size_t GetValleySamplePosition() const;

		//! Set the valley sample position.
		void SetValleySamplePosition(std::size_t valleyPoint);

		//! Get the x-axis point of where the valley is plotted.
		std::size_t GetValleyPoint() const;

		//! Get the x-axis point of where the peak is plotted.
		std::size_t GetPeakPoint() const;

		//! Add plotted point from the analysis graph.
		void PushPlottedPoint(double point);

		//! Get all plotted points used in the analysis graph.
		const std::vector<double>& GetPlottedPoints() const;

	private:
		std::size_t startSamplePoisiton_;
		std::size_t stepSize_;
		std::size_t peakSample_;
		std::size_t valleySample_;
		std::vector<double> plottedPoints_;
};

}