/*
 * PhaseVocoder
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
#include <Signal/TransientDetector.h>
#include <AudioData/AudioData.h>
#include <WaveFile/WaveFileDefines.h>
#include <WaveFile/WaveFileReader.h>

std::vector<std::size_t> GetTransientPositions(const std::string& waveFilename, double secondaryLevelThreshold=0.0)
{
	WaveFile::WaveFileReader inputWaveFile{waveFilename};

	Signal::TransientDetector transientDetector(inputWaveFile.GetSampleRate());

	if(secondaryLevelThreshold != 0.0)
	{
		transientDetector.SetValleyToPeakRatio(secondaryLevelThreshold);
	}

	std::vector<std::size_t> transients;

	transientDetector.FindTransients(inputWaveFile.GetAudioData()[WaveFile::MONO_CHANNEL], transients);

	return transients;
}

TEST(TransientDetectorTests, TestGetSetValleyToPeakRatio)
{
	Signal::TransientDetector transientDetector(44100);
	EXPECT_EQ(1.5, transientDetector.GetValleyToPeakRatio());
	transientDetector.SetValleyToPeakRatio(1.25);
	EXPECT_EQ(1.25, transientDetector.GetValleyToPeakRatio());
}

TEST(TransientDetectorTests, TestGetSetLevelStepValues)
{
	Signal::TransientDetector transientDetector(44100);

	// First check the default settings
	EXPECT_EQ(11.60998, transientDetector.GetFirstLevelStep());
	EXPECT_EQ(5.80499, transientDetector.GetSecondLevelStep());
	EXPECT_EQ(0.725623, transientDetector.GetThirdLevelStep());

	// Then check setting your own
	transientDetector.SetFirstLevelStep(5.6);
	transientDetector.SetSecondLevelStep(3.4);
	transientDetector.SetThirdLevelStep(1.2);

	EXPECT_EQ(5.6, transientDetector.GetFirstLevelStep());
	EXPECT_EQ(3.4, transientDetector.GetSecondLevelStep());
	EXPECT_EQ(1.2, transientDetector.GetThirdLevelStep());
}

TEST(TransientDetectorTests, TestSilence)
{
	AudioData audioData;
	audioData.AddSilence(10000);

	Signal::TransientDetector transientDetector(44100);
	std::vector<std::size_t> transientPositions;
	transientDetector.FindTransients(audioData, transientPositions);

	EXPECT_EQ(0, transientPositions.size());
}

TEST(TransientDetectorTests, PeakAt6000And10000)
{
	auto transientPositions{GetTransientPositions("PeakAt6000And10000.wav")};

	EXPECT_EQ(2, transientPositions.size());
	if(transientPositions.size() == 2)
	{
		EXPECT_EQ(5999, transientPositions[0]);
		EXPECT_EQ(9952, transientPositions[1]);
	}
}

TEST(TransientDetectorTests, AcousticGuitarDualStringPluck)
{
	auto transientPositions{GetTransientPositions("AcousticGuitarDualStringPluck.wav")};

	EXPECT_EQ(2, transientPositions.size());
	if(transientPositions.size() == 2)
	{
		EXPECT_EQ(24, transientPositions[0]);
		EXPECT_EQ(44096, transientPositions[1]);
	}
}

TEST(TransientDetectorTests, BuiltToSpillBeatBeginningWithSilence)
{
	auto transientPositions{GetTransientPositions("BuiltToSpillBeatBeginningWithSilence.wav")};

	EXPECT_EQ(10, transientPositions.size());
	if(transientPositions.size() == 10)
	{
		EXPECT_EQ(11026, transientPositions[0]);
		EXPECT_EQ(30944, transientPositions[1]);
		EXPECT_EQ(50912, transientPositions[2]);
		EXPECT_EQ(71104, transientPositions[3]);
		EXPECT_EQ(80864, transientPositions[4]);
		EXPECT_EQ(89088, transientPositions[5]);
		EXPECT_EQ(100256, transientPositions[6]);
		EXPECT_EQ(110944, transientPositions[7]);
		EXPECT_EQ(129600, transientPositions[8]);
		EXPECT_EQ(149184, transientPositions[9]);
	}
}

TEST(TransientDetectorTests, BuiltToSpillBeat32123HzSampleRate)
{
	auto transientPositions{GetTransientPositions("BuiltToSpillBeat32123HzSampleRate.wav")};

	EXPECT_EQ(9, transientPositions.size());
	if(transientPositions.size() == 9)
	{
		EXPECT_EQ(3, transientPositions[0]);
		EXPECT_EQ(14424, transientPositions[1]);
		EXPECT_EQ(29045, transientPositions[2]);
		EXPECT_EQ(43707, transientPositions[3]);
		EXPECT_EQ(50953, transientPositions[4]);
		EXPECT_EQ(64990, transientPositions[5]);
		EXPECT_EQ(72778, transientPositions[6]);
		EXPECT_EQ(86366, transientPositions[7]);
		EXPECT_EQ(100631, transientPositions[8]);
	}
}

TEST(TransientDetectorTests, BuiltToSpillBeatBeginningWithSilenceSpecificRatio)
{
	auto transientPositions{GetTransientPositions("BuiltToSpillBeatBeginningWithSilence.wav", 10.0)};

	EXPECT_EQ(6, transientPositions.size());
	if(transientPositions.size() == 6)
	{
		EXPECT_EQ(11026, transientPositions[0]);
		EXPECT_EQ(30464, transientPositions[1]);
		EXPECT_EQ(50912, transientPositions[2]);
		EXPECT_EQ(80864, transientPositions[3]);
		EXPECT_EQ(100256, transientPositions[4]);
		EXPECT_EQ(129600, transientPositions[5]);
	}
}

TEST(TransientDetectorTests, SweetEmotion)
{
	auto transientPositions{GetTransientPositions("SweetEmotion.wav")};

	EXPECT_EQ(8, transientPositions.size());
	if(transientPositions.size() == 10)
	{
		EXPECT_EQ(0, transientPositions[0]);
		EXPECT_EQ(28288, transientPositions[1]);
		EXPECT_EQ(56416, transientPositions[2]);
		EXPECT_EQ(84032, transientPositions[3]);
		EXPECT_EQ(97152, transientPositions[4]);
		EXPECT_EQ(111296, transientPositions[5]);
		EXPECT_EQ(125184, transientPositions[6]);
		EXPECT_EQ(139040, transientPositions[7]);
	}
}

TEST(TransientDetectorTests, SweetEmotionSpecificRatio)
{
	auto transientPositions{GetTransientPositions("SweetEmotion.wav", 0.5)};

	EXPECT_EQ(17, transientPositions.size());
	if(transientPositions.size() == 17)
	{
		EXPECT_EQ(0, transientPositions[0]);
		EXPECT_EQ(27968, transientPositions[1]);
		EXPECT_EQ(43040, transientPositions[2]);
		EXPECT_EQ(56224, transientPositions[3]);
		EXPECT_EQ(64416, transientPositions[4]);
		EXPECT_EQ(66080, transientPositions[5]);
		EXPECT_EQ(79744, transientPositions[6]);
		EXPECT_EQ(84032, transientPositions[7]);
		EXPECT_EQ(91744, transientPositions[8]);
		EXPECT_EQ(97376, transientPositions[9]);
		EXPECT_EQ(102144, transientPositions[10]);
		EXPECT_EQ(111104, transientPositions[11]);
		EXPECT_EQ(118368, transientPositions[12]);
		EXPECT_EQ(121344, transientPositions[13]);
		EXPECT_EQ(125120, transientPositions[14]);
		EXPECT_EQ(139040, transientPositions[15]);
		EXPECT_EQ(153408, transientPositions[16]);
	}
}

TEST(TransientDetectorTests, TestNonExistantPeakValleyInfo)
{
	Signal::TransientDetector transientDetector(44100);
	try
	{
		transientDetector.GetPeakAndValleyInfo(0, Signal::TransientDetector::Step::FIRST);
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Peak and valley info doesn't exist for transient|0", theException.what());
	}
}

TEST(TransientDetectorTests, TestGettingPeakAndValleyInfoAcousticGuitarDualStringPluck)
{
	WaveFile::WaveFileReader inputWaveFile{"AcousticGuitarDualStringPluck.wav"};
	Signal::TransientDetector transientDetector(inputWaveFile.GetSampleRate());

	std::vector<std::size_t> transients;
	transientDetector.FindTransients(inputWaveFile.GetAudioData()[WaveFile::MONO_CHANNEL], transients);

	EXPECT_EQ(2, transients.size());

	{
		auto peakAndValleyInfo{transientDetector.GetPeakAndValleyInfo(1, Signal::TransientDetector::Step::FIRST)};
		EXPECT_EQ(85, peakAndValleyInfo.GetValleyPoint());
		EXPECT_EQ(86, peakAndValleyInfo.GetPeakPoint());
		EXPECT_EQ(88, peakAndValleyInfo.plottedPoints_.size());
	}

	{
		auto peakAndValleyInfo{transientDetector.GetPeakAndValleyInfo(1, Signal::TransientDetector::Step::SECOND)};
		EXPECT_EQ(1, peakAndValleyInfo.GetValleyPoint());
		EXPECT_EQ(3, peakAndValleyInfo.GetPeakPoint());
		EXPECT_EQ(5, peakAndValleyInfo.plottedPoints_.size());
	}

	{
		auto peakAndValleyInfo{transientDetector.GetPeakAndValleyInfo(1, Signal::TransientDetector::Step::THIRD)};
		EXPECT_EQ(0, peakAndValleyInfo.GetValleyPoint());
		EXPECT_EQ(0, peakAndValleyInfo.GetPeakPoint());
		EXPECT_EQ(0, peakAndValleyInfo.plottedPoints_.size());
	}
}