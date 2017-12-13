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
#include <WaveFile/WaveFileWriter.h>

//! @file Writer.h
//! @brief A threadsafe audio file writer.

class AudioData;

namespace ThreadSafeAudioFile {

//! A class allowing for writing data to an audio file in a threadsafe manner.

//! Good for writing multiple channel audio file data in a multithreaded manner.

class Writer
{
	public:
		//! @brief Construct the AudioFile Writer.
		//! @param filename The name of the audio filename to create.
		//! @param channels The number of channels in the audio file.
		//! @param sampleRate The sample rate of the audio file.
		//! @param bitsPerSample The bit resolution of the audio file.
		Writer(const std::string& filename, std::size_t channels, std::size_t sampleRate, std::size_t bitsPerSample);
		virtual ~Writer();

		//! Allows for writing audio data to the audio file.
		//! @param streamID Identifies the channel to write audio data to.
		//! @param audioData The audio data to write.
		void WriteAudioStream(std::size_t streamID, const AudioData& audioData);

		//! Get the max number of buffered samples occurring while writing the audio file.
		std::size_t GetMaxBufferedSamples();

	private:
		WaveFile::WaveFileWriter waveFileWriter_;
		std::mutex mutex_;
		std::vector<AudioData> audioDataBuffers_;

		std::size_t maxBufferedSamples_{0};
};

}