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
#include <WaveFile/WaveFileDefines.h>
#include <WaveFile/WaveFileReader.h>
#include <WaveFile/WaveFileWriter.h>
#include <Utilities/Exception.h>
#include <cmath>

TEST(WaveWriterTest, TestInvalidChannels)
{
	ASSERT_THROW(WaveFile::WaveFileWriter("TestFile.wav", 3, 44100, 16), Utilities::Exception);
}

TEST(WaveWriterTest, TestInvalidBitResolution)
{
	ASSERT_THROW(WaveFile::WaveFileWriter("TestFile.wav", 1, 44100, 8), Utilities::Exception);
}

TEST(WaveWriterTest, AppendEmptyVector)
{
	// Test trying to append an empty vector of audio data (make sure it doesn't crash)
	WaveFile::WaveFileWriter waveFileWriter("TestFile.wav", 1, 44100, 16);
	waveFileWriter.AppendAudioData(std::vector<AudioData>{});
}

TEST(WaveWriterTest, AppendEmptyAudioDataToMonoChannel)
{
	// Test trying to append audio data channel of size zero (make sure it doesn't crash)
	WaveFile::WaveFileWriter waveFileWriter("TestFile.wav", 1, 44100, 16);
	AudioData audioData;
	std::vector<AudioData> audioDataVector;
	audioDataVector.push_back(audioData);
	waveFileWriter.AppendAudioData(audioDataVector);
}

TEST(WaveWriterTest, AppendEmptyAudioDataToStereoChannels)
{
	// Test trying to append audio data channel of size zero (make sure it doesn't crash)
	WaveFile::WaveFileWriter waveFileWriter("TestFile.wav", 2, 44100, 16);
	AudioData audioData;
	std::vector<AudioData> audioDataVector;
	audioDataVector.push_back(audioData);
	audioDataVector.push_back(audioData);
	waveFileWriter.AppendAudioData(audioDataVector);
}

TEST(WaveWriterTest, CreateMonoWaveFileAndVerify)
{
	const uint16_t channels{1};
	const uint32_t sampleRate{44100};
	const uint16_t bitsPerSample{16};
	const std::vector<double> rawAudioData{0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0, -0.25, -0.50, -0.75, -1.0, -0.75, -0.50, -0.25, 0.0};
	AudioData audioData{rawAudioData};

	// I scope this so that it goes out of scope (closes the file) before trying to read it
	{
		WaveFile::WaveFileWriter waveWriter{"MonoTestOutputFile.wav", channels, sampleRate, bitsPerSample};
		waveWriter.AppendAudioData(std::vector<AudioData>{audioData});
		EXPECT_EQ("MonoTestOutputFile.wav", waveWriter.GetFilename());
		EXPECT_EQ(channels, waveWriter.GetChannels());
		EXPECT_EQ(sampleRate, waveWriter.GetSampleRate());
		EXPECT_EQ(bitsPerSample, waveWriter.GetBitsPerSample());
	}

	WaveFile::WaveFileReader waveReader{"MonoTestOutputFile.wav"};

	EXPECT_EQ(bitsPerSample, waveReader.GetBitsPerSample());
	EXPECT_EQ(sampleRate * bitsPerSample / 8, waveReader.GetByteRate());
	EXPECT_EQ(channels, waveReader.GetChannels());
	EXPECT_EQ(rawAudioData.size(), waveReader.GetSampleCount());
	EXPECT_EQ(audioData.GetSize(), waveReader.GetSampleCount());
	EXPECT_EQ(sampleRate, waveReader.GetSampleRate());
	EXPECT_EQ(WaveFile::WAVE_FILE_HEADER_SIZE, waveReader.GetWaveHeaderSize());

	// Verify the data we wrote is the data that is in the file
	auto readAudioData{waveReader.GetAudioData()[WaveFile::MONO_CHANNEL].GetData()};
	for(std::size_t i{0}; i < rawAudioData.size(); ++i)
	{
		EXPECT_NEAR(rawAudioData[i], readAudioData[i], 0.0001);	
	}
}

TEST(WaveWriterTest, CreateStereoWaveFileAndVerify)
{
	const uint16_t channels{2};
	const uint32_t sampleRate{44100};
	const uint16_t bitsPerSample{16};

	// To keep the channels easily identifiable from each other I simply made them inverses of each other
	const std::vector<double> rawAudioDataLeft{0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0, -0.25, -0.50, -0.75, -1.0, -0.75, -0.50, -0.25, 0.0};
	const std::vector<double> rawAudioDataRight{0, -0.25, -0.5, -0.75, -1.0, -0.75, -0.5, -0.25, 0, 0.25, 0.50, 0.75, 1.0, 0.75, 0.50, 0.25, 0.0};

	auto audioData = std::vector<AudioData>{AudioData(rawAudioDataLeft), AudioData(rawAudioDataRight)};

	// I scope this so that it goes out of scope (closes the file) before trying to read it
	{
		WaveFile::WaveFileWriter waveWriter{"StereoTestOutputFile.wav", channels, sampleRate, bitsPerSample};
		waveWriter.AppendAudioData(audioData);
		EXPECT_EQ("StereoTestOutputFile.wav", waveWriter.GetFilename());
		EXPECT_EQ(channels, waveWriter.GetChannels());
		EXPECT_EQ(sampleRate, waveWriter.GetSampleRate());
		EXPECT_EQ(bitsPerSample, waveWriter.GetBitsPerSample());
	}

	WaveFile::WaveFileReader waveReader{"StereoTestOutputFile.wav"};

	EXPECT_EQ(bitsPerSample, waveReader.GetBitsPerSample());
	EXPECT_EQ(sampleRate * bitsPerSample / 8 * channels, waveReader.GetByteRate());
	EXPECT_EQ(channels, waveReader.GetChannels());
	EXPECT_EQ(rawAudioDataLeft.size(), waveReader.GetSampleCount());
	EXPECT_EQ(rawAudioDataRight.size(), waveReader.GetSampleCount());
	EXPECT_EQ(sampleRate, waveReader.GetSampleRate());
	EXPECT_EQ(WaveFile::WAVE_FILE_HEADER_SIZE, waveReader.GetWaveHeaderSize());

	// Verify the data we wrote is the data that is in the file
	auto readAudioData{waveReader.GetAudioData()};
	auto readAudioDataLeft{readAudioData[WaveFile::LEFT_CHANNEL].GetData()};
	auto readAudioDataRight{readAudioData[WaveFile::RIGHT_CHANNEL].GetData()};

	// Every wave file should always have the same number of samples on both channels
	EXPECT_EQ(readAudioDataLeft.size(), readAudioDataRight.size());

	// Now check the data...
	for(std::size_t i{0}; i < rawAudioDataLeft.size(); ++i)
	{
		EXPECT_NEAR(rawAudioDataLeft[i], readAudioDataLeft[i], 0.0001);	
		EXPECT_NEAR(rawAudioDataRight[i], readAudioDataRight[i], 0.0001);	
	}
}
