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

#include <Signal/Windowing.h>
#include <Utilities/Exception.h>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>

namespace Signal {

void BlackmanWindow(std::vector<double>& inputSignal, bool inverse, bool reverse, double startPercent, double endPercent)
{
	std::size_t size{inputSignal.size()};

	double bufferSizeAsDouble{static_cast<double>(size)};

	std::size_t newSize = static_cast<std::size_t>((bufferSizeAsDouble / ((endPercent - startPercent) / 100.0)) + 0.5);
	std::size_t startIndex = static_cast<std::size_t>(static_cast<double>(newSize) * (startPercent / 100.0) + 0.5);
	std::size_t endIndex = static_cast<std::size_t>(static_cast<double>(newSize) * (endPercent / 100.0) + 0.5);

	if(inputSignal.size() != (endIndex - startIndex))
	{
		Utilities::Exception("BlackmanWindow: endIndex - startIndex does not match inputSignal size");
	}

	std::size_t signalIndex{0};
	std::size_t currentIndex{startIndex};
	double twoPiDivNewSizeMinusOne{(2.0 * M_PI) / (static_cast<double>(newSize) - 1.0)};
	double fourPiDivNewSizeMinusOne{(4.0 * M_PI) / (static_cast<double>(newSize) - 1.0)};
	while(currentIndex < endIndex)
	{
		double indexAsDouble{static_cast<double>(currentIndex)};

		// See Appendix A of the Darwen Audio Phase Vocoder document for details on the Blackman window, coefficients, etc
		double amp{0.42659 - (0.49656 * cos(indexAsDouble * twoPiDivNewSizeMinusOne)) + (0.076849 * cos(indexAsDouble * fourPiDivNewSizeMinusOne))};

		if(inverse)
		{
			amp = 1.0 - amp;
		}

		if(reverse)
		{
			inputSignal[signalIndex] = inputSignal[signalIndex] / amp;
		}
		else
		{
			inputSignal[signalIndex] = inputSignal[signalIndex] * amp;
		}

		++signalIndex;
		++currentIndex;
	}
}
}

void Signal::BlackmanWindow(std::vector<double>& inputSignal, double startPercent, double endPercent)
{
	Signal::BlackmanWindow(inputSignal, false, false, startPercent, endPercent);
}

void Signal::InverseBlackmanWindow(std::vector<double>& inputSignal, double startPercent, double endPercent)
{
	Signal::BlackmanWindow(inputSignal, true, false, startPercent, endPercent);
}

void Signal::ReverseBlackmanWindow(std::vector<double>& inputSignal, double startPercent, double endPercent)
{
	Signal::BlackmanWindow(inputSignal, false, true, startPercent, endPercent);
}

void Signal::LinearFadeInOut(std::vector<double>& inputSignal)
{
	double halfBufferSizeAsDouble{static_cast<double>(inputSignal.size() - 1) / 2.0};
	std::size_t inputSignalSize{inputSignal.size()};

	for (std::size_t n{0}; n < inputSignalSize / 2; ++n)
	{
		double ampValue{static_cast<double>(n) / halfBufferSizeAsDouble};
		inputSignal[n] *= ampValue;
		inputSignal[inputSignalSize - n - 1] *= ampValue;
	}
}
