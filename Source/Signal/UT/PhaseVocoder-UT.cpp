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
#include <Signal/PhaseVocoder.h>
#include <Signal/SignalConversion.h>
#include <WaveFile/WaveFileReader.h>
#include <WaveFile/WaveFileWriter.h>
#include <Utilities/File.h>

// This performs phase vocoding on the input filename and puts result in the output filenam
void DoPhaseVocoding(const std::string& inputFilename, const std::string& outputFilename, double stretchFactor)
{
	WaveFile::WaveFileReader inputWaveFile{inputFilename};

	Signal::PhaseVocoder phaseVocoder{inputWaveFile.GetSampleRate(), inputWaveFile.GetSampleCount(), stretchFactor};

	//phaseVocoder.SubmitAudioData(AudioData(inputWaveFile.GetAudioData(), inputWaveFile.GetSampleCount()));
	phaseVocoder.SubmitAudioData(inputWaveFile.GetAudioData()[0]);

	WaveFile::WaveFileWriter waveWriter(outputFilename, inputWaveFile.GetChannels(), inputWaveFile.GetSampleRate(), inputWaveFile.GetBitsPerSample());
	waveWriter.AppendAudioData(std::vector<AudioData>{phaseVocoder.FlushAudioData()});
}

// I'm disabng these for MSVC debug builds b/c they will take minites.  Rest assured these run for release builds as they take seconds.
#ifndef _DEBUG

// Compress a single piano note
TEST(PhaseVocoderTest, TestCompressPianoNote)
{
	DoPhaseVocoding("SinglePianoKey.wav", "SinglePianoKeyCompressedCurrentResult.wav", 0.70);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("SinglePianoKeyCompressed.wav", "SinglePianoKeyCompressedCurrentResult.wav"));
}

// Stretch a single piano note
TEST(PhaseVocoderTest, TestStretchPianoNote)
{
	DoPhaseVocoding("SinglePianoKey.wav", "SinglePianoKeyStretchedCurrentResult.wav", 1.30);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("SinglePianoKeyStretched.wav", "SinglePianoKeyStretchedCurrentResult.wav"));
}

// Compress a guitar string plucked
TEST(PhaseVocoderTest, TestCompressAcousticGuitarStringPluck)
{
	DoPhaseVocoding("AcousticGuitarStringPluck.wav", "AcousticGuitarStringPluckCompressedCurrentResult.wav", 0.88);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("AcousticGuitarStringPluckCompressed.wav", "AcousticGuitarStringPluckCompressedCurrentResult.wav"));
}

// Stretch a guitar string plucked
TEST(PhaseVocoderTest, TestStretchAcousticGuitarStringPluck)
{
	DoPhaseVocoding("AcousticGuitarStringPluck.wav", "AcousticGuitarStringPluckStretchedCurrentResult.wav", 1.10);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("AcousticGuitarStringPluckStretched.wav", "AcousticGuitarStringPluckStretchedCurrentResult.wav"));
}

// Compress an 808 bass drum
TEST(PhaseVocoderTest, TestCompress808BassDrum)
{
	DoPhaseVocoding("808BassDrum.wav", "808BassDrumCompressedCurrentResult.wav", 0.60);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808BassDrumCompressed.wav", "808BassDrumCompressedCurrentResult.wav"));
}

// Stretch an 808 bass drum
TEST(PhaseVocoderTest, TestStretch808BassDrum)
{
	DoPhaseVocoding("808BassDrum.wav", "808BassDrumStretchedCurrentResult.wav", 1.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808BassDrumStretched.wav", "808BassDrumStretchedCurrentResult.wav"));
}

// Compress a one second sine wave sampled at 32768 Hz
TEST(PhaseVocoderTest, TestCompressSineWave)
{
	DoPhaseVocoding("100HzSineWaveAt32768Hz.wav", "100HzSineWaveAt32768HzCompressedCurrentResult.wav", 0.50);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("100HzSineWaveAt32768HzCompressed.wav", "100HzSineWaveAt32768HzCompressedCurrentResult.wav"));
}

// Stretch a one second sine wave sampled at 32768 Hz
TEST(PhaseVocoderTest, TestStretchSineWave)
{
	DoPhaseVocoding("100HzSineWaveAt32768Hz.wav", "100HzSineWaveAt32768HzStretchedCurrentResult.wav", 1.75);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("100HzSineWaveAt32768HzStretched.wav", "100HzSineWaveAt32768HzStretchedCurrentResult.wav"));
}

// Edge case of compressing just 10 samples
TEST(PhaseVocoderTest, TestCompress10SamplesOfInput)
{
	DoPhaseVocoding("TenSamples.wav", "TenSamplesOutputCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("TenSamplesOutputCompressed.wav", "TenSamplesOutputCompressedCurrentResult.wav"));
}

// Edge case of stretching just 10 samples
TEST(PhaseVocoderTest, TestStretch10SamplesOfInput)
{
	DoPhaseVocoding("TenSamples.wav", "TenSamplesOutputStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("TenSamplesOutputStretched.wav", "TenSamplesOutputStretchedCurrentResult.wav"));
}

// Edge case of compressing 616 samples
TEST(PhaseVocoderTest, TestCompress616SamplesOfInput)
{
	DoPhaseVocoding("808RimShot616SamplesLong.wav", "808RimShot616SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808RimShot616SamplesLongCompressed.wav", "808RimShot616SamplesLongCompressedCurrentResult.wav"));
}

// Edge case of stretching 616 samples
TEST(PhaseVocoderTest, TestStretch616SamplesOfInput)
{
	DoPhaseVocoding("808RimShot616SamplesLong.wav", "808RimShot616SamplesLongStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808RimShot616SamplesLongStretched.wav", "808RimShot616SamplesLongStretchedCurrentResult.wav"));
}

// Edge case of compressing 1024 samples
TEST(PhaseVocoderTest, TestCompress1024SamplesOfInput)
{
	DoPhaseVocoding("808Snare1024SamplesLong.wav", "808Snare1024SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare1024SamplesLongCompressed.wav", "808Snare1024SamplesLongCompressedCurrentResult.wav"));
}

// Edge case of stretching 1024 samples
TEST(PhaseVocoderTest, TestStretch1024SamplesOfInput)
{
	DoPhaseVocoding("808Snare1024SamplesLong.wav", "808Snare1024SamplesLongOutputStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare1024SamplesLongOutputStretched.wav", "808Snare1024SamplesLongOutputStretchedCurrentResult.wav"));
}

// Edge case of compressing 2615 samples
TEST(PhaseVocoderTest, TestCompress2615SamplesOfInput)
{
	DoPhaseVocoding("808Snare2615SamplesLong.wav", "808Snare2615SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare2615SamplesLongCompressed.wav", "808Snare2615SamplesLongCompressedCurrentResult.wav"));
}

// Edge case of stretching 2615 samples
TEST(PhaseVocoderTest, TestStretch2615SamplesOfInput)
{
	DoPhaseVocoding("808Snare2615SamplesLong.wav", "808Snare2615SamplesLongStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare2615SamplesLongStretched.wav", "808Snare2615SamplesLongStretchedCurrentResult.wav"));
}

// Testing compressing exactly 4096 samples (the FFT size)
TEST(PhaseVocoderTest, TestCompress4096SamplesOfInput)
{
	DoPhaseVocoding("808Snare4096SamplesLong.wav", "808Snare4096SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4096SamplesLongCompressed.wav", "808Snare4096SamplesLongCompressedCurrentResult.wav"));
}

// Testing stretching exactly 4096 samples (the FFT size)
TEST(PhaseVocoderTest, TestStretch4096SamplesOfInput)
{
	DoPhaseVocoding("808Snare4096SamplesLong.wav", "808Snare4096SamplesLongStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4096SamplesLongStretched.wav", "808Snare4096SamplesLongStretchedCurrentResult.wav"));
}

// Testing compressing 4097 samples (just over the FFT size)
TEST(PhaseVocoderTest, TestCompress4097SamplesOfInput)
{
	DoPhaseVocoding("808Snare4097SamplesLong.wav", "808Snare4097SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4097SamplesLongCompressed.wav", "808Snare4097SamplesLongCompressedCurrentResult.wav"));
}

// Testing stretching 4097 samples (just over the FFT size)
TEST(PhaseVocoderTest, TestStretch4097SamplesOfInput)
{
	DoPhaseVocoding("808Snare4097SamplesLong.wav", "808Snare4097SamplesLongStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4097SamplesLongStretched.wav", "808Snare4097SamplesLongStretchedCurrentResult.wav"));
}

// Testing compressing 4100 samples (just over the FFT size)
TEST(PhaseVocoderTest, TestCompress4100SamplesOfInput)
{
	DoPhaseVocoding("808Snare4100SamplesLong.wav", "808Snare4100SamplesLongCompressedCurrentResult.wav", 0.80);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4100SamplesLongCompressed.wav", "808Snare4100SamplesLongCompressedCurrentResult.wav"));
}

// Testing stretching 4100 samples (just over the FFT size)
TEST(PhaseVocoderTest, TestStretch4100SamplesOfInput)
{
	DoPhaseVocoding("808Snare4100SamplesLong.wav", "808Snare4100SamplesLongStretchedCurrentResult.wav", 1.20);
 	EXPECT_EQ(true, Utilities::File::CheckIfFilesMatch("808Snare4100SamplesLongStretched.wav", "808Snare4100SamplesLongStretchedCurrentResult.wav"));
}

#endif