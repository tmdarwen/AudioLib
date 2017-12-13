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
#include <ThreadSafeAudioFile/Writer.h>
#include <ThreadSafeAudioFile/UT/AudioFileDataHelper.h>
#include <AudioData/AudioData.h>
#include <WaveFile/WaveFileReader.h>
#include <Utilities/Exception.h>
#include <thread>

const std::size_t MonoAudio = 1;
const std::size_t StereoAudio = 2;
const std::size_t SampleRate = 44100;
const std::size_t BitResolution = 16;

TEST(ThreadSafeAudioFileWriter, TestInvalidChannels)
{
	ASSERT_THROW(ThreadSafeAudioFile::Writer("TestFile.wav", 3, SampleRate, BitResolution), Utilities::Exception);
}

TEST(ThreadSafeAudioFileWriter, TestInvalidBitResolution)
{
	ASSERT_THROW(ThreadSafeAudioFile::Writer("TestFile.wav", StereoAudio, SampleRate, 8), Utilities::Exception);
}

TEST(ThreadSafeAudioFileWriter, TestWritingMonoFile)
{
	AudioData audioData{CreateAudioData(44100)};	

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		ThreadSafeAudioFile::Writer writer{"TestFile.wav", MonoAudio, SampleRate, BitResolution};
		writer.WriteAudioStream(0, audioData);
		writer.WriteAudioStream(0, audioData);
		writer.WriteAudioStream(0, audioData);
	}

	// Now read the file we just wrote and confirm it contains the expected audio data
	WaveFile::WaveFileReader waveFileReader{"TestFile.wav"};
	for(std::size_t i{0}; i < 3; ++i)
	{
		auto monoChannel{audioData.GetData()};

		auto readAudioData{waveFileReader.GetAudioData(44100)};
		auto readMonoChannel{readAudioData[0].GetData()};
		for(std::size_t j{0}; j < 44100; ++j)
		{
			EXPECT_NEAR(monoChannel[j], readMonoChannel[j], 0.001);
		}
	}
}

TEST(ThreadSafeAudioFileWriter, TestWritingStereoFile)
{
	AudioData audioDataLeftChannel{CreateAudioData(44100)};
	AudioData audioDataRightChannel{CreateInvertedAudioData(44100)};

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		ThreadSafeAudioFile::Writer writer{"TestFile.wav", StereoAudio, SampleRate, BitResolution};
		writer.WriteAudioStream(0, audioDataLeftChannel);
		writer.WriteAudioStream(1, audioDataRightChannel);
	}

	// Now read the file we just wrote and confirm it contains the expected audio data

	WaveFile::WaveFileReader waveFileReader{"TestFile.wav"};
	auto readAudioData{waveFileReader.GetAudioData()};

	auto leftChannel{audioDataLeftChannel.GetData()};
	auto readLeftChannel{readAudioData[0].GetData()};

	auto rightChannel{audioDataRightChannel.GetData()};
	auto readRightChannel{readAudioData[1].GetData()};

	for(std::size_t i{0}; i < 44100; ++i)
	{
		EXPECT_NEAR(leftChannel[i], readLeftChannel[i], 0.001);
		EXPECT_NEAR(rightChannel[i], readRightChannel[i], 0.001);
	}
}

TEST(ThreadSafeAudioFileWriter, TestThreadedWritingStereoFile)
{
	AudioData audioDataLeftChannel{CreateAudioData(44100)};
	AudioData audioDataRightChannel{CreateInvertedAudioData(44100)};

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		ThreadSafeAudioFile::Writer writer{"TestFile.wav", StereoAudio, SampleRate, BitResolution};
	
		std::thread leftThread{[&] { writer.WriteAudioStream(0, audioDataLeftChannel); } };
		std::thread rightThread{[&] { writer.WriteAudioStream(1, audioDataRightChannel); } };
	
		leftThread.join();
		rightThread.join();
	}

	// Now read the file we just wrote and confirm it contains the expected audio data

	WaveFile::WaveFileReader waveFileReader{"TestFile.wav"};
	auto readAudioData{waveFileReader.GetAudioData()};

	auto leftChannel{audioDataLeftChannel.GetData()};
	auto readLeftChannel{readAudioData[0].GetData()};

	auto rightChannel{audioDataRightChannel.GetData()};
	auto readRightChannel{readAudioData[1].GetData()};

	for(std::size_t i{0}; i < 44100; ++i)
	{
		EXPECT_NEAR(leftChannel[i], readLeftChannel[i], 0.001);
		EXPECT_NEAR(rightChannel[i], readRightChannel[i], 0.001);
	}
}

TEST(ThreadSafeAudioFileWriter, TestMultipleSameChannelStereoWrites)
{
	AudioData audioData{CreateAudioData(44100)};
	AudioData audioDataInverted{CreateInvertedAudioData(44100)};

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		ThreadSafeAudioFile::Writer writer{"TestFile.wav", StereoAudio, SampleRate, BitResolution};
	
		writer.WriteAudioStream(0, audioData);
		writer.WriteAudioStream(0, audioDataInverted);
		writer.WriteAudioStream(0, audioData);
		writer.WriteAudioStream(1, audioDataInverted);
		writer.WriteAudioStream(1, audioData);
		writer.WriteAudioStream(1, audioDataInverted);
	}

	// Now read the file we just wrote and confirm it contains the expected audio data

	WaveFile::WaveFileReader waveFileReader{"TestFile.wav"};

	{
		auto readAudioData{waveFileReader.GetAudioData(44100)};
		for(std::size_t i{0}; i < 44100; ++i)
		{
			EXPECT_NEAR(audioData.GetData()[i], readAudioData[0].GetData()[i], 0.001);
			EXPECT_NEAR(audioDataInverted.GetData()[i], readAudioData[1].GetData()[i], 0.001);
		}
	}

	{
		auto readAudioData{waveFileReader.GetAudioData(44100)};
		for(std::size_t i{0}; i < 44100; ++i)
		{
			EXPECT_NEAR(audioDataInverted.GetData()[i], readAudioData[0].GetData()[i], 0.001);
			EXPECT_NEAR(audioData.GetData()[i], readAudioData[1].GetData()[i], 0.001);
		}
	}

	{
		auto readAudioData{waveFileReader.GetAudioData(44100)};
		for(std::size_t i{0}; i < 44100; ++i)
		{
			EXPECT_NEAR(audioData.GetData()[i], readAudioData[0].GetData()[i], 0.001);
			EXPECT_NEAR(audioDataInverted.GetData()[i], readAudioData[1].GetData()[i], 0.001);
		}
	}
}
