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

#pragma once

#include <cstdint>

//! @file WaveFileHeader.h
//! @brief Structure representing the a typical wave file header.

namespace WaveFile { 

//! Structure representing the a typical wave file header.

//! See http://soundfile.sapp.org/doc/WaveFormat/ for more info.

struct WaveFileHeader
{
	//! Contains the letters "RIFF".
	unsigned char chunkID_[4];

	//! Contains a numeric value of 36 + SubChunk2Size.
	unsigned char chunkSize_[4];

	//! Contains the letters "WAVE".
	unsigned char format_[4];

	//! Contains the letters "fmt "
	unsigned char subChunk1ID_[4];

	//! Contains a value of 16.
	unsigned char subChunk1Size_[4];

	//! Contains a value of 1 for PCM data.
	unsigned char audioFormat_[2];

	//! Contains the number of channels.
	unsigned char channels_[2];

	//! Contains the sample rate.
	unsigned char sampleRate_[4];

	//! Contains the byte rate: sampleRate_ * channels_ * bitsPerSample_ / 8.
	unsigned char byteRate_[4];

	//! Contains the block alignment: channels_ * bitsPerSample_ / 8.
	unsigned char blockAlign_[2];

	//! Contains bits per sample.
	unsigned char bitsPerSample_[2];

	//! Contains the letters "data".
	unsigned char subChunk2ID_[4];

	//! Number of bytes of data: samples * channels_ * bitsPerSample_ / 8.
	unsigned char subChunk2Size_[4];
};

//! Size of the wave file header in bytes.
static const std::size_t WAVE_FILE_HEADER_SIZE{44};

}
