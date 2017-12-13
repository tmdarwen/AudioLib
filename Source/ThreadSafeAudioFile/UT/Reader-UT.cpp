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
#include <ThreadSafeAudioFile/Reader.h>
#include <AudioData/AudioData.h>
#include <WaveFile/WaveFileWriter.h>
#include <Utilities/Exception.h>
#include <thread>
#include <ThreadSafeAudioFile/UT/AudioFileDataHelper.h>

const std::size_t MonoAudio = 1;
const std::size_t StereoAudio = 2;
const std::size_t SampleRate = 44100;
const std::size_t BitResolution = 16;

TEST(ThreadSafeAudioFileReader, TestNonExistanceFile)
{
	ASSERT_THROW(ThreadSafeAudioFile::Reader("WaveFileThatDoesntExist.wav"), Utilities::Exception);
}

TEST(ThreadSafeAudioFileReader, TestReadingMonoFile)
{
	AudioData audioData{CreateAudioData(44100)};	

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		WaveFile::WaveFileWriter writer{"TestFile.wav", MonoAudio, SampleRate, BitResolution};
		writer.AppendAudioData(std::vector<AudioData>{audioData});
	}

	// Now read the file we just wrote and confirm it contains the expected audio data
	ThreadSafeAudioFile::Reader reader{"TestFile.wav"};
	EXPECT_EQ(MonoAudio, reader.GetChannels());
	EXPECT_EQ(SampleRate, reader.GetSampleRate());
	EXPECT_EQ(BitResolution, reader.GetBitsPerSample());
	EXPECT_EQ(44100, reader.GetSampleCount());
	auto readAudioData{reader.ReadAudioStream(0, 0, 44100)};

	for(std::size_t i{0}; i < 44100; ++i)
	{
		EXPECT_NEAR(audioData.GetData()[i], readAudioData.GetData()[i], 0.001);
	}

	// Also try reading a specific section and confirm it's correct
	auto readAudioData2{reader.ReadAudioStream(0, 200, 10)};
	for(std::size_t i{0}; i < 10; ++i)
	{
		EXPECT_NEAR(audioData.GetData()[200 + i], readAudioData2.GetData()[i], 0.001);
	}
}

TEST(ThreadSafeAudioFileReader, TestReadingStereoFile)
{
	AudioData audioData{CreateAudioData(44100)};	
	AudioData audioDataInverted{CreateInvertedAudioData(44100)};	

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		WaveFile::WaveFileWriter writer{"TestFile.wav", StereoAudio, SampleRate, BitResolution};
		writer.AppendAudioData(std::vector<AudioData>{audioData, audioDataInverted});
	}

	// Now read the file we just wrote and confirm it contains the expected audio data
	ThreadSafeAudioFile::Reader reader{"TestFile.wav"};
	EXPECT_EQ(StereoAudio, reader.GetChannels());
	EXPECT_EQ(SampleRate, reader.GetSampleRate());
	EXPECT_EQ(BitResolution, reader.GetBitsPerSample());
	EXPECT_EQ(44100, reader.GetSampleCount());
	auto readAudioDataLeft{reader.ReadAudioStream(0, 0, 44100)};
	auto readAudioDataRight{reader.ReadAudioStream(1, 0, 44100)};

	for(std::size_t i{0}; i < 44100; ++i)
	{
		EXPECT_NEAR(audioData.GetData()[i], readAudioDataLeft.GetData()[i], 0.001);
		EXPECT_NEAR(audioDataInverted.GetData()[i], readAudioDataRight.GetData()[i], 0.001);
	}

	// Also try reading a specific section and confirm it's correct
	auto readAudioDataLeft2{reader.ReadAudioStream(0, 400, 10)};
	auto readAudioDataRight2{reader.ReadAudioStream(1, 400, 10)};
	for(std::size_t i{0}; i < 10; ++i)
	{
		EXPECT_NEAR(audioData.GetData()[400 + i], readAudioDataLeft2.GetData()[i], 0.001);
		EXPECT_NEAR(audioDataInverted.GetData()[400 + i], readAudioDataRight2.GetData()[i], 0.001);
	}
}

TEST(ThreadSafeAudioFileReader, TestReadingStereoFileThreaded)
{
	AudioData audioData{CreateAudioData(44100)};	
	AudioData audioDataInverted{CreateInvertedAudioData(44100)};	

	// Scoping the following so TestFile is closed when our writer is destroyed and we don't 
	// attempt to read the file where it might have zero size due to buffering.
	{
		WaveFile::WaveFileWriter writer{"TestFile.wav", StereoAudio, SampleRate, BitResolution};
		writer.AppendAudioData(std::vector<AudioData>{audioData, audioDataInverted});
	}

	ThreadSafeAudioFile::Reader reader{"TestFile.wav"};
	EXPECT_EQ(StereoAudio, reader.GetChannels());
	EXPECT_EQ(SampleRate, reader.GetSampleRate());
	EXPECT_EQ(BitResolution, reader.GetBitsPerSample());
	EXPECT_EQ(44100, reader.GetSampleCount());
	AudioData leftChannelRead;
	AudioData rightChannelRead;

	std::thread leftThread{[&] { leftChannelRead = reader.ReadAudioStream(0, 0, 44100); } };
	std::thread rightThread{[&] { rightChannelRead = reader.ReadAudioStream(1, 0, 44100); } };
	
	leftThread.join();
	rightThread.join();

	// Confirm the read contains the expected audio data
	for(std::size_t i{0}; i < 44100; ++i)
	{
		EXPECT_NEAR(audioData.GetData()[i], leftChannelRead.GetData()[i], 0.001);
		EXPECT_NEAR(audioDataInverted.GetData()[i], rightChannelRead.GetData()[i], 0.001);
	}
}