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

#include <Signal/PeakFrequencyDetection.h>
#include <Signal/Fourier.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <algorithm>
#include <Utilities/Exception.h>
#include <Utilities/Stringify.h>
#include <cstdint>

namespace Signal {

double Tau(double x)
{
	return (0.25 * log(3.0 * (x*x) + 6.0*x + 1.0) - sqrt(6.0)/24.0 * log((x + 1.0 - sqrt(2.0/3.0))  /  (x + 1.0 + sqrt(2.0/3.0))));
}

}

// *******************************************************************************************************
// ****************** USE THIS "QUINN" ALGORITHM AND AVOID THE CORRELATION ONE BELOW *********************
// *******************************************************************************************************
// I got this algorithm from: http://dspguru.com/dsp/howtos/how-to-interpolate-fft-peak it's called "Quinn's Second Estimator".
// If there's any question about how well it performs compared to correlation, see the UT titled "TestPeakBinFrequencyAccuracy"
// Note: real = the real portion of the frequency domain for the time domain signal
//       imaginary = the imaginary portion of the frequency domain for the time domain signal
double Signal::GetPeakFrequencyByQuinn(std::size_t peakBin, 
													std::size_t fourierSize, 
													const std::vector<double>& real, 
													const std::vector<double>& imaginary, 
													double inputSignalSampleRate)
{
	double hzPerFrequencyBin{inputSignalSampleRate / static_cast<double>(fourierSize)};

	double ap = (real[peakBin + 1] * real[peakBin] + imaginary[peakBin+1] * imaginary[peakBin])  /  (real[peakBin] * real[peakBin] + imaginary[peakBin] * imaginary[peakBin]);
	double dp = (-1.0 * ap) / (1 - ap);
	double am = (real[peakBin - 1] * real[peakBin] + imaginary[peakBin - 1] * imaginary[peakBin])  /  (real[peakBin] * real[peakBin] + imaginary[peakBin] * imaginary[peakBin]);
	double dm = am / (1 - am);
	double d = (dp + dm) / 2 + Signal::Tau(dp * dp) - Signal::Tau(dm * dm);
	double peakFrequency{(peakBin + d) * hzPerFrequencyBin};

	return peakFrequency;
}

// Let's the user just pass in a peakBin and time domain signal and internally does the FFT
double Signal::GetPeakFrequencyByQuinn(std::size_t peakBin, const std::vector<double>& timeDomainSignal, double inputSignalSampleRate)
{
	auto frequencyDomain{Signal::Fourier::ApplyFFT(AudioData(timeDomainSignal))};
	return GetPeakFrequencyByQuinn(peakBin, timeDomainSignal.size(), frequencyDomain.GetRealComponent(), frequencyDomain.GetImaginaryComponent(), inputSignalSampleRate);
}

// The phase parameter is the starting phase of the signal and can be anywhere from 0-to-360 degrees
std::vector<double> Signal::GenerateSineWave(double sampleRate, std::size_t lengthInSamples, double signalFrequency, double phase)
{
	std::vector<double> sineWave;

	double phaseFactor{2.0 * M_PI * phase / 360.0};

	double frequency{sampleRate / signalFrequency};

	double preCalculation{2.0 * M_PI / frequency};  // Doing it outside of the loop to save time

	for(double i{0.0}; i < lengthInSamples; i += 1.0)
	{
		sineWave.push_back(sin((preCalculation * i) + phaseFactor));
	}

	return sineWave;
}

// Don't use this algorithm.  Use the Quinn algorithm below.  If you have any question which performs better see the TestPeakBinFrequencyAccuracy UT
double Signal::GetPeakFrequencyByCorrelation(std::size_t peakBin, const std::vector<double>& timeDomainSignal, double inputSignalSampleRate)
{
	double fourierSize{static_cast<double>(timeDomainSignal.size())};
	double hzPerFrequencyBin{inputSignalSampleRate / fourierSize};

	double startFrequency{static_cast<double>(peakBin - 1) * hzPerFrequencyBin};
	double endFrequency{static_cast<double>(peakBin + 1) * hzPerFrequencyBin};

	double highestCorrelation{0.0};
	double highestCorrelationFrequency{0.0};

	for(double correlateFrequency{startFrequency}; correlateFrequency <= endFrequency; correlateFrequency += 0.1)
	{
		std::vector<double> correlationSignal{Signal::GenerateSineWave(inputSignalSampleRate, timeDomainSignal.size(), correlateFrequency)};

		double aTimesBSum{0};
		double aSquaredSum{0};
		double bSquaredSum{0};

		for(std::size_t i = 0; i < correlationSignal.size(); ++i)
		{
			auto a = correlationSignal[i];
			auto b = timeDomainSignal[i];

			aTimesBSum += a * b;
			aSquaredSum += a * a;
			bSquaredSum += b * b;
		}

		double correlationValue = (aTimesBSum) / sqrt(aSquaredSum * bSquaredSum);

		if(correlationValue < 0.0)
		{
			correlationValue *= -1.0;
		}
		
		if(correlationValue > highestCorrelation)
		{
			highestCorrelation = correlationValue;
			highestCorrelationFrequency = correlateFrequency;
		}
	}

	return highestCorrelationFrequency;
}

