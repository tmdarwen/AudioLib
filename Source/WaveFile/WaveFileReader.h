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
#include <WaveFile/WaveFileHeader.h>
#include <AudioData/AudioData.h>

//! @file WaveFileReader.h
//! @brief Class facilitating reading a typical wave file.

namespace WaveFile {

//! Class facilitating reading a typical wave file.

//! See http://soundfile.sapp.org/doc/WaveFormat/ for details.

class WaveFileReader
{
	public:
		//! Instantiate the object with the given wave filename.
  		WaveFileReader(const std::string& filename);
		~WaveFileReader();

		//! Gets the wave file header.
		const WaveFile::WaveFileHeader& GetHeader();

		//! Get the size of the wave file in bytes.
		std::size_t GetFileSize();

		//! Get the chunk size defined in the header.
		std::size_t GetChunkSize();

		//! Get the byte rate defined in the header.
		std::size_t GetByteRate();

		//! Get the channels contained in the wave file.
		std::size_t GetChannels();

		//! Get the sample rate of the audio file.
		std::size_t GetSampleRate();

		//! Get the bit resolution for a sample.
		std::size_t GetBitsPerSample();

		//! Get the block alignment as defined in the header.
		std::size_t GetBlockAlign();

		//! Get the sub chunk 1 size as defined in the header.
		std::size_t GetSubChunk1Size();

		//! Get the sub chunk 2 size as defined in the header.
		std::size_t GetSubChunk2Size() const;

		//! Get the wave file's header size.
		std::size_t GetWaveHeaderSize();

		//! Get the number of samples in the wave file.
		std::size_t GetSampleCount() const;

		//! Get the entire audio data content of the wave file.
		std::vector<AudioData> GetAudioData();

		//! Read the next "samplesToRead" in the wave file.
		std::vector<AudioData> GetAudioData(std::size_t samplesToRead);

		//! Read the "samplesToRead" in the wave file for a specific position.
		std::vector<AudioData> GetAudioData(std::size_t samplesStartPosition, std::size_t samplesToRead);

	private:
		void ReadHeader();
		void ValidateHeader();

		// Returns the byte position in the file where the audio sample data starts
		std::size_t GetBytePositionInFileWhereAudioSamplesStart();

		// Returns the number of bytes of audio (sample) data that exist in the file
		std::size_t GetByteCountOfSampleData();

		// Returns the position the file pointer is currently at
		std::size_t GetCurrentBytePositionOfFilePointer();

		// Returns the position the file pointer is currently at with respect to audio samples
		std::size_t GetCurrentSamplePositionOfFilePointer();

		// Sets inputFileStream_ to the given audio sample position
		void FilePointerSeekToSamplePosition(std::size_t samplesStartPosition);

		std::vector<uint8_t> Read(std::size_t bytes, std::size_t filePosition=0);

		std::string filename_;
		WaveFile::WaveFileHeader header_;
		static const unsigned int waveHeaderSize_{sizeof(WaveFile::WaveFileHeader)};
		std::size_t sampleCount_;

		std::ifstream inputFileStream_;
		void OpenFile();
};

} // End of namespace