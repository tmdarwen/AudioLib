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
#include <Signal/Resampler.h>
#include <Signal/SignalConversion.h>
#include <WaveFile/WaveFileReader.h>
#include <WaveFile/WaveFileWriter.h>
#include <Utilities/File.h>

void DoResampling(const std::string& inputFilename, const std::string& outputFilename, std::size_t newSampleRate)
{
	WaveFile::WaveFileReader inputWaveFile{inputFilename};

	double resampleRatio{static_cast<double>(newSampleRate) / static_cast<double>(inputWaveFile.GetSampleRate())};

	Signal::Resampler resampler{inputWaveFile.GetSampleRate(), resampleRatio};

	resampler.SubmitAudioData(inputWaveFile.GetAudioData()[0]);

	WaveFile::WaveFileWriter waveWriter(outputFilename, inputWaveFile.GetChannels(), newSampleRate, inputWaveFile.GetBitsPerSample());
	waveWriter.AppendAudioData(std::vector<AudioData>{resampler.FlushAudioData()});
}

TEST(ResamplerTests, SineWaveResampled)
{
	DoResampling("100HzSineWaveAt32768Hz.wav", "100HzSineWaveAt32768HzResampledCurrentResult.wav", 38000);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("100HzSineWaveAt32768HzResampled.wav", "100HzSineWaveAt32768HzResampledCurrentResult.wav"));
}

TEST(ResamplerTests, PianoKeyResampled)
{
	DoResampling("SinglePianoKey.wav", "SinglePianoKeyResampledCurrentResult.wav", 24123);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("SinglePianoKeyResampled.wav", "SinglePianoKeyResampledCurrentResult.wav"));
}

TEST(ResamplerTests, TestLowPassFilterAt2000Hz)
{
	DoResampling("400HzSineAnd2121HzSine.wav", "400HzSineAnd2121HzSineResampledCurrentResult.wav", 2000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("400HzSineAnd2121HzSineResampled.wav", "400HzSineAnd2121HzSineResampledCurrentResult.wav"));
}

TEST(ResamplerTests, TestLowPassFilterAt5000Hz)
{
	DoResampling("222HzSineAnd19000HzSine.wav", "222HzSineAnd19000HzSineResampledCurrentResult.wav", 5000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("222HzSineAnd19000HzSineResampled.wav", "222HzSineAnd19000HzSineResampledCurrentResult.wav"));
}

TEST(ResamplerTests, TestLowPassFilterAt15000Hz)
{
	DoResampling("5000HzSineAnd9797HzSine.wav", "5000HzSineAnd9797HzSineResampledCurrentResult.wav", 15000);
 	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("5000HzSineAnd9797HzSineResampled.wav", "5000HzSineAnd9797HzSineResampledCurrentResult.wav"));
}
