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

#pragma once

#include <cstddef>
#include <vector>

namespace Signal {

class TransientPeakAndValley
{
	public:
		TransientPeakAndValley(std::size_t startSamplePosition, std::size_t stepSize);

		void Reset(std::size_t startSamplePosition, std::size_t stepSize);

		std::size_t GetStartSamplePosition() const;
		std::size_t GetStepSize() const;

		std::size_t GetPeakSamplePosition() const;
		void SetPeakSamplePosition(std::size_t peakPoint);

		std::size_t GetValleySamplePosition() const;
		void SetValleySamplePosition(std::size_t valleyPoint);

		std::size_t GetValleyPoint() const;
		std::size_t GetPeakPoint() const;

		void PushPlottedPoint(double point);
		const std::vector<double>& GetPlottedPoints() const;

	private:
		std::size_t startSamplePoisiton_;
		std::size_t stepSize_;
		std::size_t peakSample_;
		std::size_t valleySample_;
		std::vector<double> plottedPoints_;
};

}