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

#include <WaveFile/WaveFileWriter.h>
#include <WaveFile/WaveFileDefines.h>
#include <WaveFile/WaveFileHeader.h>
#include <AudioData/AudioData.h>
#include <Utilities/Exception.h>
#include <Signal/SignalConversion.h>
#include <Utilities/Stringify.h>

WaveFile::WaveFileWriter::WaveFileWriter(const std::string& filename, std::size_t channels, std::size_t sampleRate, std::size_t bitsPerSample) :
	filename_{filename}, channels_{channels}, sampleRate_{sampleRate}, bitsPerSample_{bitsPerSample}, sampleCount_{0}
{
	if(channels_ != 1 && channels_ != 2)
	{
		Utilities::ThrowException(Utilities::Stringify("Attempting to instantiate WaveFileWriter with non-standard channels: ") + Utilities::Stringify(channels));
	}

	if(bitsPerSample_ != 16)
	{
		Utilities::ThrowException(Utilities::Stringify("Attempting to instantiate WaveFileWriter Writer with non-standard bit resolution: ") + Utilities::Stringify(bitsPerSample));
	}

	fileStream_.open(filename_, std::ios::out | std::ios::binary);
	if(!fileStream_.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}


	// Write some blank space for the wave header. (The actual wave header will be written when the file is closed as 
	// we don't know some of the values until we're ready to close the file).
	uint8_t blankSpace[WaveFile::WAVE_FILE_HEADER_SIZE];
 	fileStream_.write(reinterpret_cast<const char*>(blankSpace), WaveFile::WAVE_FILE_HEADER_SIZE); 
	if(!fileStream_.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to write empty header to file " + filename_));
	}
}

WaveFile::WaveFileWriter::~WaveFileWriter()
{
	WriteWaveFileHeader();

	fileStream_.close();
}

const std::string& WaveFile::WaveFileWriter::GetFilename()
{
	return filename_;
}

std::size_t WaveFile::WaveFileWriter::GetChannels()
{
	return channels_;
}

std::size_t WaveFile::WaveFileWriter::GetSampleRate()
{
	return sampleRate_;
}

std::size_t WaveFile::WaveFileWriter::GetBitsPerSample()
{
	return bitsPerSample_;
}

std::size_t WaveFile::WaveFileWriter::GetSampleCount()
{
	return sampleCount_;
}

void WaveFile::WaveFileWriter::AppendAudioData(const std::vector<AudioData>& audioData)
{
	// First make sure we've been given data to append
	if((audioData.size() == 0) || 
	(channels_ == 1 && audioData[WaveFile::MONO_CHANNEL].GetSize() == 0) || 
	(channels_ == 2 && audioData[WaveFile::LEFT_CHANNEL].GetSize() == 0 && audioData[WaveFile::RIGHT_CHANNEL].GetSize() == 0))
	{
		return;
	}

	if(audioData.size() != channels_)
	{
		Utilities::Exception(Utilities::Stringify("Given audio data to WaveFileWrite is does not correspond to specified channels " + filename_));
	}

	if(channels_ == 2 && audioData[WaveFile::LEFT_CHANNEL].GetSize() != audioData[WaveFile::RIGHT_CHANNEL].GetSize())
	{
		Utilities::Exception(Utilities::Stringify("Given stereo audio data has differing sample sizes between channels " + filename_));
	}

	std::vector<int16_t> shortOutput;

	if(channels_ == 1)
	{
		shortOutput = Signal::ConvertAudioDataToSigned16(audioData[WaveFile::MONO_CHANNEL]);	
	}
	else if(channels_ == 2)
	{
		shortOutput = Signal::ConvertAudioDataToInterleavedSigned16(audioData[WaveFile::LEFT_CHANNEL], audioData[WaveFile::RIGHT_CHANNEL]);	
	}

 	fileStream_.write(reinterpret_cast<const char*>(&shortOutput[0]), sizeof(int16_t) * shortOutput.size());
	if(!fileStream_.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to write audio data to file " + filename_));
	}

	sampleCount_ += (shortOutput.size() / channels_);
}

/*
The following info comes from http://soundfile.sapp.org/doc/WaveFormat/
The canonical WAVE format starts with the RIFF header:

0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk 
                               following this number.  This is the size of the 
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some 
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this 
                               number.
44        *   Data             The actual sound data.
*/
void WaveFile::WaveFileWriter::WriteWaveFileHeader()
{
	WaveFile::WaveFileHeader waveHeader;

	waveHeader.chunkID_[0] = 'R';
	waveHeader.chunkID_[1] = 'I';
	waveHeader.chunkID_[2] = 'F';
	waveHeader.chunkID_[3] = 'F';

	uint32_t subChunk2Size{static_cast<uint32_t>(sampleCount_) * static_cast<uint32_t>(channels_) * (static_cast<uint32_t>(bitsPerSample_) / 8)};

	uint32_t chunkSize{4 + (8 + SIZE_OF_SUBCHUNK1) + (8 + subChunk2Size)};

	waveHeader.chunkSize_[0] = chunkSize & 0xFF;
	waveHeader.chunkSize_[1] = (chunkSize >> 8) & 0xFF;
	waveHeader.chunkSize_[2] = (chunkSize >> 16) & 0xFF;
	waveHeader.chunkSize_[3] = (chunkSize >> 24) & 0xFF;

	waveHeader.format_[0] = 'W';
	waveHeader.format_[1] = 'A';
	waveHeader.format_[2] = 'V';
	waveHeader.format_[3] = 'E';

	waveHeader.subChunk1ID_[0] = 'f';
	waveHeader.subChunk1ID_[1] = 'm';
	waveHeader.subChunk1ID_[2] = 't';
	waveHeader.subChunk1ID_[3] = ' ';

	waveHeader.subChunk1Size_[0] = SIZE_OF_SUBCHUNK1 & 0xFF;
	waveHeader.subChunk1Size_[1] = (SIZE_OF_SUBCHUNK1 >> 8) & 0xFF;
	waveHeader.subChunk1Size_[2] = (SIZE_OF_SUBCHUNK1 >> 16) & 0xFF;
	waveHeader.subChunk1Size_[3] = (SIZE_OF_SUBCHUNK1 >> 24) & 0xFF;

	waveHeader.audioFormat_[0] = PCM_AUDIO_FORMAT & 0xFF;
	waveHeader.audioFormat_[1] = (PCM_AUDIO_FORMAT >> 8) & 0xFF;

	waveHeader.channels_[0] = channels_ & 0xFF;
	waveHeader.channels_[1] = (channels_ >> 8) & 0xFF;

	waveHeader.sampleRate_[0] = sampleRate_ & 0xFF;
	waveHeader.sampleRate_[1] = (sampleRate_ >> 8) & 0xFF;
	waveHeader.sampleRate_[2] = (sampleRate_ >> 16) & 0xFF;
	waveHeader.sampleRate_[3] = (sampleRate_ >> 24) & 0xFF;

	uint32_t byteRate{static_cast<uint32_t>(sampleRate_) * static_cast<uint32_t>(channels_) * (static_cast<uint32_t>(bitsPerSample_) / 8)};

	waveHeader.byteRate_[0] = byteRate & 0xFF;
	waveHeader.byteRate_[1] = (byteRate >> 8) & 0xFF;
	waveHeader.byteRate_[2] = (byteRate >> 16) & 0xFF;
	waveHeader.byteRate_[3] = (byteRate >> 24) & 0xFF;

	uint32_t blockAlign{static_cast<uint32_t>(channels_ * (bitsPerSample_ / 8))};

	waveHeader.blockAlign_[0] = blockAlign & 0xFF;
	waveHeader.blockAlign_[1] = (blockAlign >> 8) & 0xFF;

	waveHeader.bitsPerSample_[0] = bitsPerSample_ & 0xFF;
	waveHeader.bitsPerSample_[1] = (bitsPerSample_ >> 8) & 0xFF;

	waveHeader.subChunk2ID_[0] = 'd';
	waveHeader.subChunk2ID_[1] = 'a';
	waveHeader.subChunk2ID_[2] = 't';
	waveHeader.subChunk2ID_[3] = 'a';

	waveHeader.subChunk2Size_[0] = subChunk2Size & 0xFF;
	waveHeader.subChunk2Size_[1] = (subChunk2Size >> 8) & 0xFF;
	waveHeader.subChunk2Size_[2] = (subChunk2Size >> 16) & 0xFF;
	waveHeader.subChunk2Size_[3] = (subChunk2Size >> 24) & 0xFF;

	fileStream_.seekp(0);
 	fileStream_.write(reinterpret_cast<const char*>(&waveHeader),WaveFile::WAVE_FILE_HEADER_SIZE);
	if(!fileStream_.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to write wave header to file " + filename_));
	}
}
