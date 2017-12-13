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

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

//! @file WaveFileWriter.h
//! @brief Class facilitating writing a typical wave file.

class AudioData;

namespace WaveFile {

//! Class facilitating reading a typical wave file.

//! See http://soundfile.sapp.org/doc/WaveFormat/ for details.

class WaveFileWriter
{
	public:
		//! @brief Instantiate the WaveFileWriter.
		//! @param filename Name of the wave file to create.
		//! @param channels The number of channels in the wave file.
		//! @param sampleRate The sample rate of the wave file.
		//! @param bitsPerSample The bit resolution of samples in the wave file.
		WaveFileWriter(const std::string& filename, std::size_t channels, std::size_t sampleRate, std::size_t bitsPerSample);
		~WaveFileWriter();

		//! @brief Write audio to the end of the wave file.
		//! An AudioData object holds a "stream" of audio.  Think of a "stream" as a single channel.  In order to write multiple 
		//! channels (i.e. a stereo file) place multiple AudioData objects in the vector.  For stereo, we follow the normal 
		//! convention of index zero being the left channel and index one being the right channel.
		void AppendAudioData(const std::vector<AudioData>& audioData);

		//! Get the current size (in samples) of the wave file.
		std::size_t GetSampleCount();

		//! Get the name of the wave file.
		const std::string& GetFilename();

		//! Get the number of channels in the wave file.
		std::size_t GetChannels();

		//! Get the sample rate of the wave file.
		std::size_t GetSampleRate();

		//! Get the bit resolution of the wave file.
		std::size_t GetBitsPerSample();

	private:
		void WriteWaveFileHeader();

		std::string filename_;
		std::size_t channels_;
		std::size_t sampleRate_;
		std::size_t bitsPerSample_;
		std::size_t sampleCount_;
		std::ofstream fileStream_;

		static const uint32_t SIZE_OF_SUBCHUNK1{16};
		static const uint16_t PCM_AUDIO_FORMAT{1};
};

}