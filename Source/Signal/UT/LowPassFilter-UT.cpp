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
#include <Signal/LowPassFilter.h>
#include <Utilities/Exception.h>
#include <Utilities/File.h>
#include <WaveFile/WaveFileDefines.h>
#include <WaveFile/WaveFileReader.h>
#include <WaveFile/WaveFileWriter.h>

void DoLowPassFiltering(const std::string& inputFilename, const std::string& outputFilename, std::size_t cutoffFrequency)
{
	WaveFile::WaveFileReader inputWaveFile{inputFilename};

	double cutoffRatio{static_cast<double>(cutoffFrequency) / static_cast<double>(inputWaveFile.GetSampleRate())};
	Signal::LowPassFilter lowPassFilter{cutoffRatio};

	lowPassFilter.SubmitAudioData(inputWaveFile.GetAudioData()[WaveFile::MONO_CHANNEL]);

	WaveFile::WaveFileWriter waveWriter(outputFilename, inputWaveFile.GetChannels(), inputWaveFile.GetSampleRate(), inputWaveFile.GetBitsPerSample());
	waveWriter.AppendAudioData(std::vector<AudioData>{lowPassFilter.FlushAudioData()});
}

TEST(LowPassFilterTests, TestInvalidCutoff)
{
	EXPECT_THROW(Signal::LowPassFilter(0.000001), Utilities::Exception);
	EXPECT_THROW(Signal::LowPassFilter(0.6), Utilities::Exception);
}

TEST(LowPassFilterTests, TestMinimumSamplesNeededForProcessing)
{
	{
		Signal::LowPassFilter lowPassFilter{0.25};
		EXPECT_EQ(100, lowPassFilter.MinimumSamplesNeededForProcessing());
	}

	{
		Signal::LowPassFilter lowPassFilter{0.25, 125};
		EXPECT_EQ(125, lowPassFilter.MinimumSamplesNeededForProcessing());
	}

	{
		Signal::LowPassFilter lowPassFilter{0.25, 75};
		EXPECT_EQ(75, lowPassFilter.MinimumSamplesNeededForProcessing());
	}
}

TEST(LowPassFilterTests, LowPassAt1000Hz)
{
	DoLowPassFiltering("400HzSineAnd2121HzSine.wav", "400HzSineAnd2121HzSineLowPassFilteredAt1000HzCurrentResults.wav", 1000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("400HzSineAnd2121HzSineLowPassFilteredAt1000Hz.wav", 
															"400HzSineAnd2121HzSineLowPassFilteredAt1000HzCurrentResults.wav"));
}

TEST(LowPassFilterTests, LowPassAt6000Hz)
{
	DoLowPassFiltering("5000HzSineAnd9797HzSine.wav", "5000HzSineAnd9797HzSineLowPassFilteredAt6000HzCurrentResults.wav", 6000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("5000HzSineAnd9797HzSineLowPassFilteredAt6000Hz.wav", 
															"5000HzSineAnd9797HzSineLowPassFilteredAt6000HzCurrentResults.wav"));
}

TEST(LowPassFilterTests, LowPassAt8000Hz)
{
	DoLowPassFiltering("222HzSineAnd19000HzSine.wav", "222HzSineAnd19000HzSineLowPassFilteredAt8000HzCurrentResults.wav", 8000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("222HzSineAnd19000HzSineLowPassFilteredAt8000Hz.wav", 
															"222HzSineAnd19000HzSineLowPassFilteredAt8000HzCurrentResults.wav"));
}