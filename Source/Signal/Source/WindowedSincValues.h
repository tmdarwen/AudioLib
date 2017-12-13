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

//! @file WindowedSincValues.h
//! @brief Lookup table for windowed sinc values.

#pragma once

#include <cstddef>

namespace Signal {

	const double SINC_SAMPLES_PER_X_INTEGER{224.0};
	const std::size_t SINC_VALUE_SIZE{9857};  //4928 on both sides plus 1.0 in the middle
	const std::size_t SINC_CENTER_POINT{4928};
	const double MAX_X_POSITION_POSITIVE{4928.0};
	const double MIN_X_POSITION_POSITIVE{-4928.0};

	//! Get the sinc value for the given x-axis position.
	//
	//! See chapter 16 of "The Scientists' and Engineers Guide to DSP" for more on the "Windowed-Sinc Filters"
	double GetSincValue(double xPosition);

}
