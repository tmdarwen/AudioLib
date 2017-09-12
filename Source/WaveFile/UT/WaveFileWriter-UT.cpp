#include <gtest/gtest.h>
#include <WaveFile/WaveFileWriter.h>
#include <WaveFile/WaveFileReader.h>
#include <Utilities/Exception.h>
#include <cmath>

TEST(WaveWriterTest, CreateWaveFileAndVerify)
{
	const uint16_t channels{1};
	const uint32_t sampleRate{44100};
	const uint16_t bitsPerSample{16};
	const std::vector<double> audioData{0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0, -0.25, -0.50, -0.75, -1.0, -0.75, -0.50, -0.25, 0.0};

	{
		WaveFile::WaveFileWriter waveWriter{"TestOutputFile.wav", channels, sampleRate, bitsPerSample};
		waveWriter.AppendAudioData(audioData);
	}

	WaveFile::WaveFileReader waveReader{"TestOutputFile.wav"};

	EXPECT_EQ(bitsPerSample, waveReader.GetBitsPerSample());
	EXPECT_EQ(sampleRate * bitsPerSample / 8, waveReader.GetByteRate());
	EXPECT_EQ(channels, waveReader.GetChannels());
	EXPECT_EQ(audioData.size(), waveReader.GetSampleCount());
	EXPECT_EQ(sampleRate, waveReader.GetSampleRate());
	EXPECT_EQ(WaveFile::WAVE_FILE_HEADER_SIZE, waveReader.GetWaveHeaderSize());

	// Verify the data we wrote is the data that is in the file
	auto readAudioData{waveReader.GetAudioData()};
	for(std::size_t i{0}; i < audioData.size(); ++i)
	{
		EXPECT_NEAR(audioData[i], readAudioData[i], 0.0001);	
	}
}
