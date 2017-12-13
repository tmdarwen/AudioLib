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

#include <Signal/Fourier.h>
#include <Utilities/Exception.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstdint>

bool Signal::Fourier::IsPowerOfTwo(std::size_t number)
{
	while(((number & 1) == 0) && number > 1)
	{
		number >>= 1;
	}

	if(number == 1)
	{
		return true;
	}

	return false;
}

Signal::FrequencyDomain Signal::Fourier::ApplyDFT(const AudioData& timeDomainSignal)
{
	/////////////////////////////////////////////////////////////////
	// Calculate the DFT using the Analysis Equation (Eq 8-4 from "The Scientist and Engineer's Guide to Digital Signal Processing")

	std::size_t N{timeDomainSignal.GetSize()};
	std::size_t K{N / 2};

	std::vector<Signal::FrequencyBin> rectangularValues;

	// First initialize reX and imX values to zeroes
	for (std::size_t k = 0; k <= K; ++k)
	{
		Signal::FrequencyBin frequencyDomainBinValues;
		frequencyDomainBinValues.reX_ = 0.0;
		frequencyDomainBinValues.imX_ = 0.0;
		rectangularValues.push_back(frequencyDomainBinValues);
	}

	auto timeDomain{timeDomainSignal.GetData()};

	for (std::size_t i = 0; i < N; ++i)  // Remember, i runs from 0 to N-1
	{
		for (std::size_t k = 0; k <= K; ++k)  // Remember, k runs from 0 to N/2
		{
			rectangularValues[k].reX_ += timeDomain[i] * cos(2.0 * M_PI * (double)k * (double)i / (double)N);
			rectangularValues[k].imX_ += -1.0 * timeDomain[i] * sin(2.0 * M_PI * (double)k * (double)i / (double)N);
		}
	}

	return Signal::FrequencyDomain{rectangularValues};
}

AudioData Signal::Fourier::ApplyInverseDFT(const Signal::FrequencyDomain& frequencyDomainData)
{
	std::size_t N{(frequencyDomainData.GetSize() - 1) * 2}; //FOURIER_SIZE;
	std::size_t K{N / 2};

	///////////////////////////////////////////////////////////////
	// Perform the normalization (See equation 8-3 "The Scientist and Engineer's Guide to Digital Signal Processing")

	std::vector<double> reXNormalized;
	std::vector<double> imXNormalized;

	for (uint64_t k = 0; k <= K; ++k)
	{
		reXNormalized.push_back(frequencyDomainData.GetBin(k).reX_ / (static_cast<double>(N) / 2.0));
		imXNormalized.push_back(-1.0 * frequencyDomainData.GetBin(k).imX_ / (static_cast<double>(N) / 2.0));
	}

	// Handle the two special cases
	reXNormalized[0] = frequencyDomainData.GetBin(0).reX_ / static_cast<double>(N);
	reXNormalized[N / 2] = frequencyDomainData.GetBin(N / 2).reX_ / static_cast<double>(N);

	///////////////////////////////////////////////////////////////
	// Perform the iDFT (See equation 8-2 "The Scientist and Engineer's Guide to Digital Signal Processing")

	AudioData audioData;

	for(uint64_t i = 0; i < N; ++i)
	{
		double currentSample{0.0};
		for(uint64_t k = 0; k <= K; ++k)
		{
			currentSample += (reXNormalized[k] * cos(2.0 * M_PI * static_cast<double>(k) * static_cast<double>(i) / static_cast<double>(N))) +
				(imXNormalized[k] * sin(2.0 * M_PI * static_cast<double>(k) * static_cast<double>(i) / static_cast<double>(N)));
		}

		audioData.PushSample(currentSample);
	}

	return audioData;
}

// The following is the FFT from program 12-4 of "The Scientist and Engineer's Guide to Digital Signal Processing"
// See Figure 12-1 in "The Scientist and Engineer's Guide to Digital Signal Processing" to understand the input/output.
// NOTE: IT HAS GOTO'S IN IT - I WOULD NEVER USE GOTO'S. NOT MY CODE, I JUST COPIED IT VERBATIM AS AN FFT IS NOT 
// TRIVIAL TO CREATE.
void ScientistsAndEngineersFFT(std::vector<double>& real, std::vector<double>& imaginary)
{

	// Preconditions for the FFT:
	assert(real.size() == imaginary.size());
	assert(Signal::Fourier::IsPowerOfTwo(real.size()));
	assert(Signal::Fourier::IsPowerOfTwo(imaginary.size()));

	if(real.size() == imaginary.size())
	{
		Utilities::Exception("ScientistsAndEngineersFFT: Given real and imaginary container sizes do not match");
	}

	if(!Signal::Fourier::IsPowerOfTwo(real.size()) || !Signal::Fourier::IsPowerOfTwo(imaginary.size()))
	{
		Utilities::Exception("ScientistsAndEngineersFFT: Real and/or imaginary signal container size is not a power of two");
	}

	// And now, the FFT (Standard disclaimer: I wanted to make this algorithm identical to what is in the book, this 
	// is why you'll see a few goto statements.  I wouldn't normally use gotos.)

	std::size_t N{real.size()};

	double tr = 0;  // Needed for declaration of tr
	double ti = 0;  // Needed for declaration of ti
	uint64_t k = 0;  // Needed for declaration of k

	auto nm1 = N - 1;
	auto nd2 = N / 2;

	uint64_t m = static_cast<uint64_t>((log((double)N) / log(2.0)));

	uint64_t j = static_cast<uint64_t>(nd2);

	// The Bit Reversal Sorting happens first  
	for(uint64_t i = 1; i <= N - 2; ++i)
	{	
		if(i >= j) goto GOTO1190;

		tr = real[j];
		ti = imaginary[j];
		real[j] = real[i];
		imaginary[j] = imaginary[i];
		real[i] = tr;
		imaginary[i] = ti;

GOTO1190:
		k = static_cast<uint64_t>(nd2);

GOTO1200:
		if(k > j) goto GOTO1240;

		j = j - k;
		k = k / 2;
		goto GOTO1200;

GOTO1240:
		j = j + k;
	}


	for(uint64_t l = 1; l <= m; ++l)  // Loop for each "stage"
	{
		uint64_t le = static_cast<uint64_t>(pow(2.0, (double)l));
		double le2 = ((double)le / 2.0);

		double ur = 1.0;
		double ui = 0.0;

		double sr = cos(M_PI / le2);
		double si = -1.0 * sin(M_PI / le2);

		for(uint64_t j = 1; j <= le2; ++j)  // Loop for each "sub DFT"
		{
			uint64_t jm1 = j - 1;
			for(uint64_t i = jm1; i <= nm1; i += le)  // Loop for each "butterfly"
			{
				uint64_t ip = i + static_cast<uint64_t>(le2);

				tr = real[ip] * ur - imaginary[ip] * ui;
				ti = real[ip] * ui + imaginary[ip] * ur;

				real[ip] = real[i] - tr;
				imaginary[ip] = imaginary[i] - ti;

				real[i] = real[i] + tr;
				imaginary[i] = imaginary[i] + ti;
			}

			tr = ur;
			ur = tr * sr - ui * si;
			ui = tr * si + ui * sr;
		}
	}
}

// The following is the FFT from program 12-5 of "The Scientist and Engineer's Guide to Digital Signal Processing"
void ScientistsAndEngineersInverseFFT(std::vector<double>& real, std::vector<double>& imaginary)
{
	// Preconditions for the FFT:
	assert(real.size() == imaginary.size());
	assert(Signal::Fourier::IsPowerOfTwo(real.size()));
	assert(Signal::Fourier::IsPowerOfTwo(imaginary.size()));

	std::for_each(imaginary.begin(), imaginary.end(), [](double& value) { value *= -1; });

	ScientistsAndEngineersFFT(real, imaginary);

	assert(real.size() == imaginary.size());

	double N{static_cast<double>(real.size())};
	std::for_each(real.begin(), real.end(), [=](double& value) { value = value / N; });
	std::for_each(imaginary.begin(), imaginary.end(), [=](double& value) { value = value / N; });
}

Signal::FrequencyDomain Signal::Fourier::ApplyFFT(const AudioData& timeDomainSignal)
{
	std::vector<double> real;
	std::vector<double> imaginary;

	for(auto sample : timeDomainSignal.GetData())
	{
		real.push_back(sample);
	}

	imaginary.resize(real.size(), 0.0);

	ScientistsAndEngineersFFT(real, imaginary);

	Signal::FrequencyDomain frequencyDomain;
	for(auto index{0}; index <= (real.size() / 2); ++index) 
	{
		frequencyDomain.PushFrequencyBin(Signal::FrequencyBin{real[index], imaginary[index]});
	}

	return frequencyDomain;
}

AudioData Signal::Fourier::ApplyInverseFFT(const Signal::FrequencyDomain& frequencyDomainData)
{
	std::vector<double> real;
	std::vector<double> imaginary;

	// See the middle of page 227 of "The Scientist and Engineer's Guide to Digital Signal Processing" for what we're doing from here...

	auto frequencyBinData{frequencyDomainData.GetRectangularFrequencyData()};

	for(auto frequencyBin : frequencyBinData)
	{
		real.push_back(frequencyBin.reX_);
		imaginary.push_back(frequencyBin.imX_);
	}

	for(std::size_t i = frequencyBinData.size() - 2; i > 0; --i)
	{
		real.push_back(frequencyBinData[i].reX_);
		imaginary.push_back(-1.0 * frequencyBinData[i].imX_);  // Note the -1 multiplication
	}

	// ...to here.

	ScientistsAndEngineersInverseFFT(real, imaginary);

	AudioData audioData;
	for(std::size_t index{0}; index < real.size(); ++index) 
	{
		audioData.PushSample(real[index]);	
	}

	return audioData;	
}
