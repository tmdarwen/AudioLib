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
#include <Utilities/Exception.h>
#include <WaveFile/WaveFileDefines.h>
#include <WaveFile/WaveFileReader.h>
#include <cmath>

TEST(WaveReaderTests, OpenFile)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
}

TEST(WaveReaderTests, FailedOpenFile)
{
	ASSERT_THROW(WaveFile::WaveFileReader("NonExistantWaveFile.wav"), Utilities::Exception);
}

TEST(WaveReaderTests, WaveFileSize)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	EXPECT_EQ(88244, waveReader.GetFileSize());
}

TEST(WaveReaderTests, ValidateReadData)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();
	EXPECT_EQ('R', header.chunkID_[0]);
	EXPECT_EQ('I', header.chunkID_[1]);
	EXPECT_EQ('F', header.chunkID_[2]);
	EXPECT_EQ('F', header.chunkID_[3]);
}

TEST(WaveReaderTests, ValidateFormat)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();
	EXPECT_EQ('W', header.format_[0]);
	EXPECT_EQ('A', header.format_[1]);
	EXPECT_EQ('V', header.format_[2]);
	EXPECT_EQ('E', header.format_[3]);
}

TEST(WaveReaderTests, ValidateChunkSize)
{
	// The document states the chunk size should be all of the following:
	// 1) 36 + SubChunk2Size
	// 2) 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
	// 3) The size of the entire file minus 8 bytes

	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();

	unsigned int headerChunkSize = header.chunkSize_[3];
	headerChunkSize <<= 8;
	headerChunkSize += header.chunkSize_[2];
	headerChunkSize <<= 8;
	headerChunkSize += header.chunkSize_[1];
	headerChunkSize <<= 8;
	headerChunkSize += header.chunkSize_[0];

	unsigned int subChunk1Size = header.subChunk1Size_[3];
	subChunk1Size <<= 8;
	subChunk1Size += header.subChunk1Size_[2];
	subChunk1Size <<= 8;
	subChunk1Size += header.subChunk1Size_[1];
	subChunk1Size <<= 8;
	subChunk1Size += header.subChunk1Size_[0];

	unsigned int subChunk2Size = header.subChunk2Size_[3];
	subChunk2Size <<= 8;
	subChunk2Size += header.subChunk2Size_[2];
	subChunk2Size <<= 8;
	subChunk2Size += header.subChunk2Size_[1];
	subChunk2Size <<= 8;
	subChunk2Size += header.subChunk2Size_[0];

	// Case 1: ChunkSize = 36 + SubChunk2Size
	// SubChunk2Size is said to be at bytes 40-43
	EXPECT_EQ(subChunk2Size + 36, headerChunkSize);

	// Case 2: ChunkSize = 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
	// SubChunk1Size is said to be at bytes 16-20
	// SubChunk2Size is said to be at bytes 40-43
	EXPECT_EQ(4 + (8 + subChunk1Size ) + (8 + subChunk2Size), headerChunkSize);

	// Case 3: ChunkSize = The size of the entire file minus 8 bytes
	EXPECT_EQ(waveReader.GetFileSize() - 8, headerChunkSize);

	// Also validate GetChunkSize() returns the same value we've retrieved
	EXPECT_EQ(waveReader.GetChunkSize(), headerChunkSize);
}

TEST(WaveReaderTests, ValidateSubchunkID)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();
	EXPECT_EQ('f', header.subChunk1ID_[0]);
	EXPECT_EQ('m', header.subChunk1ID_[1]);
	EXPECT_EQ('t', header.subChunk1ID_[2]);
	EXPECT_EQ(' ', header.subChunk1ID_[3]);
}

TEST(WaveReaderTests, AudioFormat)
{
	// Audio format is said to have a value of 1 for PCM.  A
	// value other than 1 indicates a form of compression.
	// We'll error on those for now as we only support PCM
	// currently.
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();
	unsigned int audioFormat = (header.audioFormat_[1] << 8) + header.audioFormat_[0];
	EXPECT_EQ(1, audioFormat);
}

TEST(WaveReaderTests, Channels)
{
	// The channel count is at bytes 22-23.  We're only going to 
	// support mono (1) and stereo (2) for now.
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();

	int channels = (header.channels_[1] << 8) + header.channels_[0];
	
	EXPECT_GT(channels, 0);  // Translates to "expect audioFormat > 0
	EXPECT_LT(channels, 3);  // Translates to "expect audioFormat < 3

	EXPECT_GT(waveReader.GetChannels(), static_cast<unsigned int>(0)); 
	EXPECT_LT(waveReader.GetChannels(), static_cast<unsigned int>(3));

	EXPECT_EQ(waveReader.GetChannels(), channels);  // We expect both values should match
}

TEST(WaveReaderTests, SampleRate)
{
	// This is a tough one to write a test for.  Typically the sample rate will be from 
	// say 8000 Hz to 192 KHz, (so 8,000 to 192,000), but theoritically the sample rate could
	// be between 1 and ??? skys the limit.
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	EXPECT_GT(waveReader.GetSampleRate(), static_cast<unsigned int>(0));
	EXPECT_LT(waveReader.GetSampleRate(), static_cast<unsigned int>(pow(2, 32) - 1));
}

TEST(WaveReaderTests, BitsPerSample)
{
	// This will usually be 8, 16, 24 or 32 but might also be 64
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	EXPECT_TRUE(waveReader.GetBitsPerSample() == 8 ||
				waveReader.GetBitsPerSample() == 16 ||
				waveReader.GetBitsPerSample() == 24 ||
				waveReader.GetBitsPerSample() == 32 ||
				waveReader.GetBitsPerSample() == 64);
}


TEST(WaveReaderTests, ByteRate)
{
	// The byte rate is at bytes 28-31.  And is said to be equal to
	// SampleRate * NumChannels * BitsPerSample/8
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	EXPECT_EQ(waveReader.GetSampleRate() * waveReader.GetChannels() * waveReader.GetBitsPerSample() / 8, waveReader.GetByteRate());
}

TEST(WaveReaderTests, BlockAlign)
{
	// According to the spec, BlockAlign are bytes 32-33 and are 
	// equal to Channels * BitsPerSample/8
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	EXPECT_EQ(waveReader.GetChannels() * waveReader.GetBitsPerSample()/8, waveReader.GetBlockAlign());
}

TEST(WaveReaderTests, ValidateSubchunk2ID)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto header = waveReader.GetHeader();
	EXPECT_EQ('d', header.subChunk2ID_[0]);
	EXPECT_EQ('a', header.subChunk2ID_[1]);
	EXPECT_EQ('t', header.subChunk2ID_[2]);
	EXPECT_EQ('a', header.subChunk2ID_[3]);
}

TEST(WaveReaderTests, ValidateSubchunk2Size)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");

	// From the spec, we can easily conclude that this should be the size 
	// of the file minus the size of the wave header.
	EXPECT_EQ(waveReader.GetFileSize() - waveReader.GetWaveHeaderSize(), waveReader.GetSubChunk2Size());
}

TEST(WaveReaderTests, GetAudioData)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto audioData{waveReader.GetAudioData()[WaveFile::MONO_CHANNEL].GetData()};

	EXPECT_EQ(waveReader.GetSampleCount(), waveReader.GetSubChunk2Size() / 2);

	EXPECT_NEAR(0.00003, audioData[0], 0.0001);
	EXPECT_NEAR(0.03140, audioData[1], 0.0001);
	EXPECT_NEAR(0.06259, audioData[2], 0.0001);

	// TestWaveFileMono.wav is a sine wave and I wanted to test a negative value so I'm
	// skipping down to sample 59
	EXPECT_NEAR(-0.26495, audioData[59], 0.0001);
}

// The left channel of the TestWaveFileStereo.wav is the same as TestWaveFileMono.wav and 
// the right channel is the same audio inverted.
TEST(WaveReaderTests, GetStereoAudioData)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");
	auto audioData{waveReader.GetAudioData()};

	auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
	EXPECT_NEAR(0.00003, audioDataLeft[0], 0.0001);
	EXPECT_NEAR(0.03140, audioDataLeft[1], 0.0001);
	EXPECT_NEAR(0.06259, audioDataLeft[2], 0.0001);
	EXPECT_NEAR(-0.26495, audioDataLeft[59], 0.0001);

	auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
	EXPECT_NEAR(0.00003, audioDataRight[0], 0.0001);
	EXPECT_NEAR(-0.03140, audioDataRight[1], 0.0001);
	EXPECT_NEAR(-0.06259, audioDataRight[2], 0.0001);
	EXPECT_NEAR(0.26495, audioDataRight[59], 0.0001);
}

TEST(WaveReaderTests, GetSpecificCountOfAudioData)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto audioData{waveReader.GetAudioData(3)[WaveFile::MONO_CHANNEL].GetData()};

	EXPECT_EQ(3, audioData.size());

	if(audioData.size() == 3)
	{
		EXPECT_NEAR(0.00003, audioData[0], 0.0001);
		EXPECT_NEAR(0.03140, audioData[1], 0.0001);
		EXPECT_NEAR(0.06259, audioData[2], 0.0001);
	}
}

TEST(WaveReaderTests, GetSpecificCountOfStereoAudioData)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");
	auto audioData{waveReader.GetAudioData(3)};

	auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};

	EXPECT_EQ(3, audioDataLeft.size());

	if(audioDataLeft.size() == 3)
	{
		EXPECT_NEAR(0.00003, audioDataLeft[0], 0.0001);
		EXPECT_NEAR(0.03140, audioDataLeft[1], 0.0001);
		EXPECT_NEAR(0.06259, audioDataLeft[2], 0.0001);
	}

	auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};

	EXPECT_EQ(3, audioDataRight.size());

	if(audioDataRight.size() == 3)
	{
		EXPECT_NEAR(-0.00003, audioDataRight[0], 0.0001);
		EXPECT_NEAR(-0.03140, audioDataRight[1], 0.0001);
		EXPECT_NEAR(-0.06259, audioDataRight[2], 0.0001);
	}
}

TEST(WaveReaderTests, GetAudioDataFromParticularLocation)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");
	auto audioData{waveReader.GetAudioData(43298, 3)[WaveFile::MONO_CHANNEL].GetData()};

	EXPECT_EQ(3, audioData.size());

	if(audioData.size() == 3)
	{
		EXPECT_NEAR(-0.0057983, audioData[0], 0.0001);
		EXPECT_NEAR(0.0256653, audioData[1], 0.0001);
		EXPECT_NEAR(0.0569458, audioData[2], 0.0001);
	}
}

TEST(WaveReaderTests, GetStereoAudioDataFromParticularLocation)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");
	auto audioData{waveReader.GetAudioData(43298, 3)};

	auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};

	EXPECT_EQ(3, audioDataLeft.size());

	if(audioDataLeft.size() == 3)
	{
		EXPECT_NEAR(-0.0057983, audioDataLeft[0], 0.0001);
		EXPECT_NEAR(0.0256653, audioDataLeft[1], 0.0001);
		EXPECT_NEAR(0.0569458, audioDataLeft[2], 0.0001);
	}

	auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};

	EXPECT_EQ(3, audioDataRight.size());

	if(audioDataRight.size() == 3)
	{
		EXPECT_NEAR(0.0057983, audioDataRight[0], 0.0001);
		EXPECT_NEAR(-0.0256653, audioDataRight[1], 0.0001);
		EXPECT_NEAR(-0.0569458, audioDataRight[2], 0.0001);
	}
}

TEST(WaveReaderTests, GetAudioDataSuccessiveReads)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");

	{
		auto audioData{waveReader.GetAudioData(10)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(10, audioData.size());
		if(audioData.size() == 10)
		{
			EXPECT_NEAR(0.00003, audioData[0], 0.0001);
			EXPECT_NEAR(0.03140, audioData[1], 0.0001);
			EXPECT_NEAR(0.06259, audioData[2], 0.0001);
			EXPECT_NEAR(0.09360, audioData[3], 0.0001);
			EXPECT_NEAR(0.12433, audioData[4], 0.0001);
			EXPECT_NEAR(0.15451, audioData[5], 0.0001);
			EXPECT_NEAR(0.18405, audioData[6], 0.0001);
			EXPECT_NEAR(0.21286, audioData[7], 0.0001);
			EXPECT_NEAR(0.24091, audioData[8], 0.0001);
			EXPECT_NEAR(0.26794, audioData[9], 0.0001);
		}
	}

	{
		auto audioData{waveReader.GetAudioData(10)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(10, audioData.size());
		if(audioData.size() == 10)
		{
			EXPECT_NEAR(0.29395, audioData[0], 0.0001);
			EXPECT_NEAR(0.31885, audioData[1], 0.0001);
			EXPECT_NEAR(0.34244, audioData[2], 0.0001);
			EXPECT_NEAR(0.36469, audioData[3], 0.0001);
			EXPECT_NEAR(0.38550, audioData[4], 0.0001);
			EXPECT_NEAR(0.40482, audioData[5], 0.0001);
			EXPECT_NEAR(0.42252, audioData[6], 0.0001);
			EXPECT_NEAR(0.43857, audioData[7], 0.0001);
			EXPECT_NEAR(0.45291, audioData[8], 0.0001);
			EXPECT_NEAR(0.46545, audioData[9], 0.0001);
		}
	}
}

TEST(WaveReaderTests, GetStereoAudioDataSuccessiveReads)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");

	{
		auto audioData{waveReader.GetAudioData(10)};

		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataLeft.size());
		if(audioDataLeft.size() == 10)
		{
			EXPECT_NEAR(0.00003, audioDataLeft[0], 0.0001);
			EXPECT_NEAR(0.03140, audioDataLeft[1], 0.0001);
			EXPECT_NEAR(0.06259, audioDataLeft[2], 0.0001);
			EXPECT_NEAR(0.09360, audioDataLeft[3], 0.0001);
			EXPECT_NEAR(0.12433, audioDataLeft[4], 0.0001);
			EXPECT_NEAR(0.15451, audioDataLeft[5], 0.0001);
			EXPECT_NEAR(0.18405, audioDataLeft[6], 0.0001);
			EXPECT_NEAR(0.21286, audioDataLeft[7], 0.0001);
			EXPECT_NEAR(0.24091, audioDataLeft[8], 0.0001);
			EXPECT_NEAR(0.26794, audioDataLeft[9], 0.0001);
		}

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataRight.size());
		if(audioDataRight.size() == 10)
		{
			EXPECT_NEAR(-0.00003, audioDataRight[0], 0.0001);
			EXPECT_NEAR(-0.03140, audioDataRight[1], 0.0001);
			EXPECT_NEAR(-0.06259, audioDataRight[2], 0.0001);
			EXPECT_NEAR(-0.09360, audioDataRight[3], 0.0001);
			EXPECT_NEAR(-0.12433, audioDataRight[4], 0.0001);
			EXPECT_NEAR(-0.15451, audioDataRight[5], 0.0001);
			EXPECT_NEAR(-0.18405, audioDataRight[6], 0.0001);
			EXPECT_NEAR(-0.21286, audioDataRight[7], 0.0001);
			EXPECT_NEAR(-0.24091, audioDataRight[8], 0.0001);
			EXPECT_NEAR(-0.26794, audioDataRight[9], 0.0001);
		}
	}

	{
		auto audioData{waveReader.GetAudioData(10)};

		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataLeft.size());
		if(audioDataLeft.size() == 10)
		{
			EXPECT_NEAR(0.29395, audioDataLeft[0], 0.0001);
			EXPECT_NEAR(0.31885, audioDataLeft[1], 0.0001);
			EXPECT_NEAR(0.34244, audioDataLeft[2], 0.0001);
			EXPECT_NEAR(0.36469, audioDataLeft[3], 0.0001);
			EXPECT_NEAR(0.38550, audioDataLeft[4], 0.0001);
			EXPECT_NEAR(0.40482, audioDataLeft[5], 0.0001);
			EXPECT_NEAR(0.42252, audioDataLeft[6], 0.0001);
			EXPECT_NEAR(0.43857, audioDataLeft[7], 0.0001);
			EXPECT_NEAR(0.45291, audioDataLeft[8], 0.0001);
			EXPECT_NEAR(0.46545, audioDataLeft[9], 0.0001);
		}

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataRight.size());
		if(audioDataRight.size() == 10)
		{
			EXPECT_NEAR(-0.29395, audioDataRight[0], 0.0001);
			EXPECT_NEAR(-0.31885, audioDataRight[1], 0.0001);
			EXPECT_NEAR(-0.34244, audioDataRight[2], 0.0001);
			EXPECT_NEAR(-0.36469, audioDataRight[3], 0.0001);
			EXPECT_NEAR(-0.38550, audioDataRight[4], 0.0001);
			EXPECT_NEAR(-0.40482, audioDataRight[5], 0.0001);
			EXPECT_NEAR(-0.42252, audioDataRight[6], 0.0001);
			EXPECT_NEAR(-0.43857, audioDataRight[7], 0.0001);
			EXPECT_NEAR(-0.45291, audioDataRight[8], 0.0001);
			EXPECT_NEAR(-0.46545, audioDataRight[9], 0.0001);
		}
	}
}

TEST(WaveReaderTests, GetAudioDataParticularPointSuccessiveReads)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");

	{
		auto audioData{waveReader.GetAudioData(6690, 10)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(10, audioData.size());
		if(audioData.size() == 10)
		{
			EXPECT_NEAR(-0.50116, audioData[0], 0.0001);
			EXPECT_NEAR(-0.50049, audioData[1], 0.0001);
			EXPECT_NEAR(-0.49789, audioData[2], 0.0001);
			EXPECT_NEAR(-0.49332, audioData[3], 0.0001);
			EXPECT_NEAR(-0.48679, audioData[4], 0.0001);
			EXPECT_NEAR(-0.47842, audioData[5], 0.0001);
			EXPECT_NEAR(-0.46808, audioData[6], 0.0001);
			EXPECT_NEAR(-0.45593, audioData[7], 0.0001);
			EXPECT_NEAR(-0.44205, audioData[8], 0.0001);
			EXPECT_NEAR(-0.42633, audioData[9], 0.0001);
		}
	}
	{
		auto audioData{waveReader.GetAudioData(20)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(20, audioData.size());

		if(audioData.size() == 20)
		{
			EXPECT_NEAR(-0.40903, audioData[0], 0.0001);
			EXPECT_NEAR(-0.39005, audioData[1], 0.0001);
			EXPECT_NEAR(-0.36963, audioData[2], 0.0001);
			EXPECT_NEAR(-0.34766, audioData[3], 0.0001);
			EXPECT_NEAR(-0.32434, audioData[4], 0.0001);
			EXPECT_NEAR(-0.29984, audioData[5], 0.0001);
			EXPECT_NEAR(-0.27408, audioData[6], 0.0001);
			EXPECT_NEAR(-0.24719, audioData[7], 0.0001);
			EXPECT_NEAR(-0.21945, audioData[8], 0.0001);
			EXPECT_NEAR(-0.19080, audioData[9], 0.0001);
			EXPECT_NEAR(-0.16134, audioData[10], 0.0001);
			EXPECT_NEAR(-0.13132, audioData[11], 0.0001);
			EXPECT_NEAR(-0.10080, audioData[12], 0.0001);
			EXPECT_NEAR(-0.06982, audioData[13], 0.0001);
			EXPECT_NEAR(-0.03851, audioData[14], 0.0001);
			EXPECT_NEAR(-0.00720, audioData[15], 0.0001);
			EXPECT_NEAR(0.024200, audioData[16], 0.0001);
			EXPECT_NEAR(0.055481, audioData[17], 0.0001);
			EXPECT_NEAR(0.086670, audioData[18], 0.0001);
			EXPECT_NEAR(0.117371, audioData[19], 0.0001);
		}
	}
}

TEST(WaveReaderTests, GetStereoAudioDataParticularPointSuccessiveReads)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");

	{
		auto audioData{waveReader.GetAudioData(6690, 10)};
		EXPECT_EQ(2, audioData.size());

		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataLeft.size());
		if(audioData.size() == 10)
		{
			EXPECT_NEAR(-0.50116, audioDataLeft[0], 0.0001);
			EXPECT_NEAR(-0.50049, audioDataLeft[1], 0.0001);
			EXPECT_NEAR(-0.49789, audioDataLeft[2], 0.0001);
			EXPECT_NEAR(-0.49332, audioDataLeft[3], 0.0001);
			EXPECT_NEAR(-0.48679, audioDataLeft[4], 0.0001);
			EXPECT_NEAR(-0.47842, audioDataLeft[5], 0.0001);
			EXPECT_NEAR(-0.46808, audioDataLeft[6], 0.0001);
			EXPECT_NEAR(-0.45593, audioDataLeft[7], 0.0001);
			EXPECT_NEAR(-0.44205, audioDataLeft[8], 0.0001);
			EXPECT_NEAR(-0.42633, audioDataLeft[9], 0.0001);
		}

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(10, audioDataRight.size());
		if(audioData.size() == 10)
		{
			EXPECT_NEAR(0.50116, audioDataRight[0], 0.0001);
			EXPECT_NEAR(0.50049, audioDataRight[1], 0.0001);
			EXPECT_NEAR(0.49789, audioDataRight[2], 0.0001);
			EXPECT_NEAR(0.49332, audioDataRight[3], 0.0001);
			EXPECT_NEAR(0.48679, audioDataRight[4], 0.0001);
			EXPECT_NEAR(0.47842, audioDataRight[5], 0.0001);
			EXPECT_NEAR(0.46808, audioDataRight[6], 0.0001);
			EXPECT_NEAR(0.45593, audioDataRight[7], 0.0001);
			EXPECT_NEAR(0.44205, audioDataRight[8], 0.0001);
			EXPECT_NEAR(0.42633, audioDataRight[9], 0.0001);
		}
	}

	{
		auto audioData{waveReader.GetAudioData(20)};
		EXPECT_EQ(2, audioData.size());

		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(20, audioDataLeft.size());
		if(audioDataLeft.size() == 20)
		{
			EXPECT_NEAR(-0.40903, audioDataLeft[0], 0.0001);
			EXPECT_NEAR(-0.39005, audioDataLeft[1], 0.0001);
			EXPECT_NEAR(-0.36963, audioDataLeft[2], 0.0001);
			EXPECT_NEAR(-0.34766, audioDataLeft[3], 0.0001);
			EXPECT_NEAR(-0.32434, audioDataLeft[4], 0.0001);
			EXPECT_NEAR(-0.29984, audioDataLeft[5], 0.0001);
			EXPECT_NEAR(-0.27408, audioDataLeft[6], 0.0001);
			EXPECT_NEAR(-0.24719, audioDataLeft[7], 0.0001);
			EXPECT_NEAR(-0.21945, audioDataLeft[8], 0.0001);
			EXPECT_NEAR(-0.19080, audioDataLeft[9], 0.0001);
			EXPECT_NEAR(-0.16134, audioDataLeft[10], 0.0001);
			EXPECT_NEAR(-0.13132, audioDataLeft[11], 0.0001);
			EXPECT_NEAR(-0.10080, audioDataLeft[12], 0.0001);
			EXPECT_NEAR(-0.06982, audioDataLeft[13], 0.0001);
			EXPECT_NEAR(-0.03851, audioDataLeft[14], 0.0001);
			EXPECT_NEAR(-0.00720, audioDataLeft[15], 0.0001);
			EXPECT_NEAR(0.024200, audioDataLeft[16], 0.0001);
			EXPECT_NEAR(0.055481, audioDataLeft[17], 0.0001);
			EXPECT_NEAR(0.086670, audioDataLeft[18], 0.0001);
			EXPECT_NEAR(0.117371, audioDataLeft[19], 0.0001);
		}

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(20, audioDataRight.size());

		if(audioDataRight.size() == 20)
		{
			EXPECT_NEAR(0.40903, audioDataRight[0], 0.0001);
			EXPECT_NEAR(0.39005, audioDataRight[1], 0.0001);
			EXPECT_NEAR(0.36963, audioDataRight[2], 0.0001);
			EXPECT_NEAR(0.34766, audioDataRight[3], 0.0001);
			EXPECT_NEAR(0.32434, audioDataRight[4], 0.0001);
			EXPECT_NEAR(0.29984, audioDataRight[5], 0.0001);
			EXPECT_NEAR(0.27408, audioDataRight[6], 0.0001);
			EXPECT_NEAR(0.24719, audioDataRight[7], 0.0001);
			EXPECT_NEAR(0.21945, audioDataRight[8], 0.0001);
			EXPECT_NEAR(0.19080, audioDataRight[9], 0.0001);
			EXPECT_NEAR(0.16134, audioDataRight[10], 0.0001);
			EXPECT_NEAR(0.13132, audioDataRight[11], 0.0001);
			EXPECT_NEAR(0.10080, audioDataRight[12], 0.0001);
			EXPECT_NEAR(0.06982, audioDataRight[13], 0.0001);
			EXPECT_NEAR(0.03851, audioDataRight[14], 0.0001);
			EXPECT_NEAR(0.00720, audioDataRight[15], 0.0001);
			EXPECT_NEAR(-0.024200, audioDataRight[16], 0.0001);
			EXPECT_NEAR(-0.055481, audioDataRight[17], 0.0001);
			EXPECT_NEAR(-0.086670, audioDataRight[18], 0.0001);
			EXPECT_NEAR(-0.117371, audioDataRight[19], 0.0001);
		}
	}
}

TEST(WaveReaderTests, GetAudioDataTestEOF)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileMono.wav");

	{
		auto audioData{waveReader.GetAudioData(44095, 10)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(5, audioData.size());

		if(audioData.size() == 5)
		{
			EXPECT_NEAR(-0.15460, audioData[0], 0.0001);
			EXPECT_NEAR(-0.12445, audioData[1], 0.0001);
			EXPECT_NEAR(-0.09372, audioData[2], 0.0001);
			EXPECT_NEAR(-0.06268, audioData[3], 0.0001);
			EXPECT_NEAR(-0.03149, audioData[4], 0.0001);
		}
	}

	// The TestWaveFile has 44100 samples in it.  Once we've read to the end-of-data we 
	// should no longer get any additional data on subsequent reads.
	{
		auto audioData{waveReader.GetAudioData(10)[WaveFile::MONO_CHANNEL].GetData()};
		EXPECT_EQ(0, audioData.size());
	}
}

TEST(WaveReaderTests, GetStereoAudioDataTestEOF)
{
	WaveFile::WaveFileReader waveReader("TestWaveFileStereo.wav");

	{
		auto audioData{waveReader.GetAudioData(44095, 10)};
		EXPECT_EQ(2, audioData.size());  // We expect two channels of data (i.e. stereo)

		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(5, audioDataLeft.size());
		if(audioData.size() == 5)
		{
			EXPECT_NEAR(0.15460, audioDataLeft[0], 0.0001);
			EXPECT_NEAR(0.12445, audioDataLeft[1], 0.0001);
			EXPECT_NEAR(0.09372, audioDataLeft[2], 0.0001);
			EXPECT_NEAR(0.06268, audioDataLeft[3], 0.0001);
			EXPECT_NEAR(0.03149, audioDataLeft[4], 0.0001);
		}

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(5, audioDataRight.size());
		if(audioData.size() == 5)
		{
			EXPECT_NEAR(-0.15460, audioDataRight[0], 0.0001);
			EXPECT_NEAR(-0.12445, audioDataRight[1], 0.0001);
			EXPECT_NEAR(-0.09372, audioDataRight[2], 0.0001);
			EXPECT_NEAR(-0.06268, audioDataRight[3], 0.0001);
			EXPECT_NEAR(-0.03149, audioDataRight[4], 0.0001);
		}
	}

	// The TestWaveFile has 44100 samples in it.  Once we've read to the end-of-data we 
	// should no longer get any additional data on subsequent reads.
	{
		auto audioData{waveReader.GetAudioData(10)};
		EXPECT_EQ(2, audioData.size());  // We expect two channels...

		// ...but both channels should have zero samples.
		auto audioDataLeft{audioData[WaveFile::LEFT_CHANNEL].GetData()};
		EXPECT_EQ(0, audioDataLeft.size());

		auto audioDataRight{audioData[WaveFile::RIGHT_CHANNEL].GetData()};
		EXPECT_EQ(0, audioDataRight.size());
	}
}
