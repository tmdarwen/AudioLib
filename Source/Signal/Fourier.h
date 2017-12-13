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

//! @file Fourier.h
//! @brief Functions allowing for Fourier transformations.

#pragma once

#include <AudioData/AudioData.h>
#include <Signal/FrequencyDomain.h>
#include <vector>

namespace Signal  {


namespace Fourier
{
	//! Returns true if the given number is a power of two; false otherwise.
	bool IsPowerOfTwo(std::size_t number);

	//! Applies the Discrete Fourier Transform to the given audio data.
	Signal::FrequencyDomain ApplyDFT(const AudioData& timeDomainSignal);

	//! Applies the Inverse Discrete Fourier Transform to the given frequency data.
	AudioData ApplyInverseDFT(const Signal::FrequencyDomain& frequencyDomainData);

	//! Applies the Fast Fourier Transform to the given audio data.
	//
	//! Note that the length of the given audio must be a power of two.
	Signal::FrequencyDomain ApplyFFT(const AudioData& timeDomainSignal);

	//! Applies the Inverse Fast Fourier Transform to the given audio data.
	//
	//! Note that the length of the given frequency domain data must be a power of two.
	AudioData ApplyInverseFFT(const Signal::FrequencyDomain& frequencyDomainData);
}

}
