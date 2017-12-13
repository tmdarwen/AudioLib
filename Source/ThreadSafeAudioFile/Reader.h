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
#include <mutex>
#include <WaveFile/WaveFileReader.h>
#include <AudioData/AudioData.h>

//! @file Reader.h
//! @brief A threadsafe audio file reader.

namespace ThreadSafeAudioFile {

//! A class allowing for reading data from an audio file in a threadsafe manner.

class Reader
{
	public:
		//! Construct the threadsafe audio file reader, taking the audio file name to read.
		Reader(const std::string& filename);
		virtual ~Reader();

		//! Get the number of channels the audio file contains.
		std::size_t GetChannels();

		//! Get the sample rate of the audio file.
		std::size_t GetSampleRate();

		//! Get the bit resolution of the audio file.
		std::size_t GetBitsPerSample();

		//! Get the number of sample in the audio file.
		std::size_t GetSampleCount();

		//! @brief Get the number of sample in the audio file.
		//! @param streamID Identifies the channel.
		//! @param sampleStartPosition The sample number of where the read should start.
		//! @param samplesToRead The number of samples to read.
		AudioData ReadAudioStream(std::size_t streamID, std::size_t sampleStartPosition, std::size_t samplesToRead);

	private:
		std::mutex mutex_;
		WaveFile::WaveFileReader waveFileReader_;
};

}