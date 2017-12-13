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

#include <gtest/gtest.h>
#include <Signal/PeakFrequencyDetection.h>
#include <Signal/SignalConversion.h>
#include <vector>

TEST(FourierTransformTests, TestPeakBinRightOfCenterByCorrelation)
{
	// We generate a 77 Hz signal 4096 samples in length, with a sample rate of 44100 Hz.
	double signalFrequency{77.0};
	std::size_t windowSize{4096};
	double signalSampleRate{44100};

	auto signal{Signal::GenerateSineWave(signalSampleRate, windowSize, signalFrequency)};

	// With 10.7 Hz per bin, the 77Hz signal should be peaked in bin 7 with bin 8 being higher magnitude than bin 6.
	std::size_t peakBin{static_cast<std::size_t>((signalFrequency / (signalSampleRate / static_cast<double>(windowSize))) + 0.5)};
	EXPECT_EQ(7, peakBin);

	// Calculate and check peak values
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByCorrelation(peakBin, signal, signalSampleRate), 0.1);
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByQuinn(peakBin, signal, signalSampleRate), 0.1);
}

TEST(FourierTransformTests, TestPeakBinLeftOfCenterByCorrelation)
{
	// We generate a 115 Hz signal 4096 samples in length, with a sample rate of 44100 Hz.
	double signalFrequency{115.0};
	std::size_t windowSize{4096};
	double signalSampleRate{44100};

	auto signal{Signal::GenerateSineWave(signalSampleRate, windowSize, signalFrequency)};

	// With 10.7 Hz per bin, the 115Hz signal should be peaked in bin 11 with bin 10 being higher magnitude than bin 12.
	std::size_t peakBin{static_cast<std::size_t>((signalFrequency / (signalSampleRate / static_cast<double>(windowSize))) + 0.5)};
	EXPECT_EQ(11, peakBin);

	// Calculate and check peak values
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByCorrelation(peakBin, signal, signalSampleRate), 0.1);
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByQuinn(peakBin, signal, signalSampleRate), 0.1);
}

TEST(FourierTransformTests, TestPeakBinAtCenterByCorrelation)
{
	// We generate a 80 Hz signal 4096 samples in length, with a sample rate of 32768 Hz.
	double signalFrequency{80.0};
	std::size_t windowSize{4096};
	double signalSampleRate{32768};

	auto signal{Signal::GenerateSineWave(signalSampleRate, windowSize, signalFrequency)};

	// With 8 Hz per bin, the 80Hz signal should be centered over bin 10.
	std::size_t peakBin{static_cast<std::size_t>((signalFrequency / (signalSampleRate / static_cast<double>(windowSize))) + 0.5)};
	EXPECT_EQ(10, peakBin);

	// Calculate and check peak values
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByCorrelation(peakBin, signal, signalSampleRate), 0.1);
	EXPECT_NEAR(signalFrequency, Signal::GetPeakFrequencyByQuinn(peakBin, signal, signalSampleRate), 0.1);
}

// The following UT shows the accuracy of the Correlation peak detector verse Quinn.  Quinn is much more accurate.  I've 
// commented out this UT b/c it'll take a couple minutes to run.  It take a 4096 window of a 102 Hz signal and continually 
// shifts it left by one sample performing both peak detections and gathering the values to calculate some stats to show 
// the accuracy of both.  An example of the output:
// ------------------------------------------------------------------------------------------------
// Correlation 16 Bit Averages: 102.093
// Correlation 64 Bit Averages: 102.093
// Quinn 16 Bit Averages: 101.998
// Quinn 64 Bit Averages: 101.998
// ------------------------------------------------------------------------------------------------
// Correlation 16 Bit Average Variance: 2.01274
// Correlation 64 Bit Average Variance: 2.01274
// Quinn 16 Bit Average Variance: 0.0924165
// Quinn 64 Bit Average Variance: 0.0924165
// ------------------------------------------------------------------------------------------------
// Correlation 16 Bit Max Diff: 4.13281
// Correlation 64 Bit Max Diff: 4.13281
// Quinn 16 Bit Max Diff: 0.146781
// Quinn 64 Bit Max Diff: 0.146781
// ------------------------------------------------------------------------------------------------
/*
TEST(FourierTransformTests, TestPeakBinFrequencyAccuracy)
{
	// We generate a 102 Hz signal 4096 samples in length, with a sample rate of 44100 Hz.
	double signalFrequency{102.0};
	std::size_t windowSize{4096};
	double signalSampleRate{44100};

	// We use a 4096 FFT on 102 Hz signal that has a sample rate of 44100 Hz.  We  should have frequency bins 
	// of 10.7666 Hz per bin causing the peak frequency bin to be 9 (9.47 rounded to the closest whole number).
	std::size_t peakBin{static_cast<std::size_t>((signalFrequency / (signalSampleRate / static_cast<double>(windowSize))) + 0.5)};
	EXPECT_EQ(9, peakBin);

	// We ceate a signal of 2x the windowSize because we're going to perform the peak calculation on it 
	// at every point across the window - therefore shifting it by a full window size at the end.
	auto testSignal{Signal::GenerateSineWave(signalSampleRate, 2*windowSize, signalFrequency)};

	std::vector<double> peakFrequenciesCorrelation16Bit;
	std::vector<double> peakFrequenciesCorrelation64Bit;
	std::vector<double> peakFrequenciesQuinn16Bit;
	std::vector<double> peakFrequenciesQuinn64Bit;

	auto displacement{windowSize};
	while(--displacement)
	{
		std::cout << displacement << std::endl;
		testSignal.erase(testSignal.begin()); // Shift the test signal to the left by one sample

		std::vector<double> signal64BitResolution{testSignal.begin(), testSignal.begin() + windowSize};
		auto tempSignal{Signal::ConvertFloat64ToSigned16(signal64BitResolution, windowSize)};
		auto signal16BitResolution{Signal::ConvertSigned16ToFloat64(tempSignal)};

		peakFrequenciesCorrelation16Bit.push_back(Signal::GetPeakFrequencyByCorrelation(peakBin, signal16BitResolution, signalSampleRate));
		peakFrequenciesCorrelation64Bit.push_back(Signal::GetPeakFrequencyByCorrelation(peakBin, signal64BitResolution, signalSampleRate));
		peakFrequenciesQuinn16Bit.push_back(Signal::GetPeakFrequencyByQuinn(peakBin, signal16BitResolution, signalSampleRate));
		peakFrequenciesQuinn64Bit.push_back(Signal::GetPeakFrequencyByQuinn(peakBin, signal64BitResolution, signalSampleRate));
	}

	auto averagePeakValue{[](std::vector<double>& peaks) { 
		double total{0.0};
		for(auto peak : peaks)
		{
			total += peak;	
		}
		return total / peaks.size();
	}};

	std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
	std::cout << "Correlation 16 Bit Averages: " << averagePeakValue(peakFrequenciesCorrelation16Bit) << std::endl;
	std::cout << "Correlation 64 Bit Averages: " << averagePeakValue(peakFrequenciesCorrelation64Bit) << std::endl;
	std::cout << "Quinn 16 Bit Averages: " << averagePeakValue(peakFrequenciesQuinn16Bit) << std::endl;
	std::cout << "Quinn 64 Bit Averages: " << averagePeakValue(peakFrequenciesQuinn64Bit) << std::endl;

	auto averageDiff{[](double signalFrequency, std::vector<double>& peaks) { 
		double total{0.0};
		for(auto peak : peaks)
		{
			double diff{peak - signalFrequency};
			if(diff < 0.0) diff *= -1.0;
			total += diff;	
		}
		return total / peaks.size();
	}};

	std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
	std::cout << "Correlation 16 Bit Average Variance: " << averageDiff(signalFrequency, peakFrequenciesCorrelation16Bit) << std::endl;
	std::cout << "Correlation 64 Bit Average Variance: " << averageDiff(signalFrequency, peakFrequenciesCorrelation64Bit) << std::endl;
	std::cout << "Quinn 16 Bit Average Variance: " << averageDiff(signalFrequency, peakFrequenciesQuinn16Bit) << std::endl;
	std::cout << "Quinn 64 Bit Average Variance: " << averageDiff(signalFrequency, peakFrequenciesQuinn64Bit) << std::endl;

	auto maxDiff{[](double signalFrequency, std::vector<double>& peaks) { 
		double maxDiff{0.0};
		for(auto peak : peaks)
		{
			double diff{peak - signalFrequency};
			if(diff < 0.0) diff *= -1.0;
			if(diff > maxDiff) maxDiff = diff;
		}
		return maxDiff;
	}};

	std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
	std::cout << "Correlation 16 Bit Max Diff: " << maxDiff(signalFrequency, peakFrequenciesCorrelation16Bit) << std::endl;
	std::cout << "Correlation 64 Bit Max Diff: " << maxDiff(signalFrequency, peakFrequenciesCorrelation64Bit) << std::endl;
	std::cout << "Quinn 16 Bit Max Diff: " << maxDiff(signalFrequency, peakFrequenciesQuinn16Bit) << std::endl;
	std::cout << "Quinn 64 Bit Max Diff: " << maxDiff(signalFrequency, peakFrequenciesQuinn64Bit) << std::endl;
	std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}
*/