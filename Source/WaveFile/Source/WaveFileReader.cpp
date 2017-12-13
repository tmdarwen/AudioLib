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

#include <WaveFile/WaveFileReader.h>
#include <Signal/SignalConversion.h>
#include <Utilities/Exception.h>
#include <cstdint>

 WaveFile::WaveFileReader::WaveFileReader(const std::string& filename) : filename_{filename}
{
	ReadHeader();
	ValidateHeader();
	OpenFile();
}

WaveFile::WaveFileReader::~WaveFileReader()
{
	inputFileStream_.close();
}

void WaveFile::WaveFileReader::ReadHeader()
{
	std::ifstream fileStream(filename_, std::ios::in | std::ios::binary);
	if(!fileStream.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}

	// Reinterpret cast here because ifstream's read wants a char* but we want to keep
	// our uint8_ts we read as unsigned chars (the WaveFileReader::uint8_t type).
	fileStream.read(reinterpret_cast<char*>(&header_), waveHeaderSize_);
	if(!fileStream.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to read wave file header " + filename_));
	}

	sampleCount_ = GetSubChunk2Size() / GetChannels() / (GetBitsPerSample()/8);

	fileStream.close();
}

void WaveFile::WaveFileReader::OpenFile()
{
	inputFileStream_.open(filename_, std::ios::in | std::ios::binary);
	if(!inputFileStream_.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}

	inputFileStream_.seekg(GetBytePositionInFileWhereAudioSamplesStart(), std::ios_base::beg);
}

std::vector<uint8_t> WaveFile::WaveFileReader::Read(std::size_t uint8_ts, std::size_t filePosition)
{
	std::vector<uint8_t> data(uint8_ts);

	std::ifstream fileStream(filename_, std::ios::in | std::ios::binary);
	if(!fileStream.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}

	fileStream.seekg(filePosition, std::ios_base::beg);

	// Reinterpret cast here because ifstream's read wants a char* but we want to keep
	// our uint8_ts we read as unsigned chars (the WaveReeader::uint8_t type).
	fileStream.read(reinterpret_cast<char*>(data.data()), uint8_ts);
	if(!fileStream.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to read audio data from wave file " + filename_));
	}

	fileStream.close();

	return data;
 }

void WaveFile::WaveFileReader::ValidateHeader()
{
	if(header_.chunkID_[0] != 'R' || header_.chunkID_[1] != 'I' || header_.chunkID_[2] != 'F' || header_.chunkID_[3] != 'F')
	{
		Utilities::ThrowException("Invalid wave header - chunk ID", filename_, __FILE__, __LINE__);
	}

	if(header_.format_[0] != 'W' || header_.format_[1] != 'A' || header_.format_[2] != 'V' || header_.format_[3] != 'E')
	{
		Utilities::ThrowException("Invalid wave header - format", filename_, __FILE__, __LINE__);
	}

	if(header_.subChunk1ID_[0] != 'f' || header_.subChunk1ID_[1] != 'm' || header_.subChunk1ID_[2] != 't' || header_.subChunk1ID_[3] != ' ')
	{
		Utilities::ThrowException("Invalid wave header - sub chunk ID 1", filename_, __FILE__, __LINE__);
	}

	if(header_.subChunk2ID_[0] != 'd' || header_.subChunk2ID_[1] != 'a' || header_.subChunk2ID_[2] != 't' || header_.subChunk2ID_[3] != 'a')
	{
		Utilities::ThrowException("Invalid wave header - sub chunk ID 2", filename_, __FILE__, __LINE__);
	}

	if(header_.audioFormat_[0] != 1 || header_.audioFormat_[1] != 0)
	{
		Utilities::ThrowException("Invalid wave header - audio format", filename_, __FILE__, __LINE__);
	}

	if(GetChannels() != 1 && GetChannels() != 2)
	{
		Utilities::ThrowException("Invalid wave header - channels", filename_, GetChannels(), __FILE__, __LINE__);
	}

	if(GetByteRate() != (GetSampleRate() * GetChannels() * GetBitsPerSample() / 8))
	{
		Utilities::ThrowException("Invalid wave header - uint8_t rate", filename_, GetByteRate(), GetSampleRate(), GetChannels(), GetBitsPerSample(), __FILE__, __LINE__);
	}

	if(GetBlockAlign() != (GetChannels() * GetBitsPerSample() / 8))
	{
		Utilities::ThrowException("Invalid wave header - uint8_t alignment", filename_, GetBlockAlign(), GetChannels(), GetBitsPerSample(), __FILE__, __LINE__);
	}

	if(GetBitsPerSample() != 8 &&
	   GetBitsPerSample() != 16 &&
	   GetBitsPerSample() != 24 &&
	   GetBitsPerSample() != 32 &&
	   GetBitsPerSample() != 64)
	{
		Utilities::ThrowException("Invalid wave header - bits per sample", filename_, GetBitsPerSample(), __FILE__, __LINE__);
	}

	if(GetBitsPerSample() != 16)
	{
		Utilities::ThrowException("Invalid wave file.  WaveFileReader class currently only supports 16 bit audio ", filename_);
	}

}

std::size_t WaveFile::WaveFileReader::GetFileSize()
{
	std::ifstream fileStream(filename_, std::ios::in | std::ios::binary);

	if(!fileStream.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}

	fileStream.seekg(0, std::ios_base::end);
	std::size_t fileSize = static_cast<std::size_t>(fileStream.tellg());
	fileStream.close();

	return fileSize;
}

std::size_t WaveFile::WaveFileReader::GetChannels()
{
	return ((header_.channels_[1] << 8) + header_.channels_[0]);
}

std::size_t WaveFile::WaveFileReader::GetSampleRate()
{
	return (header_.sampleRate_[3] << 24) + (header_.sampleRate_[2] << 16) + (header_.sampleRate_[1] << 8) + header_.sampleRate_[0];
}

std::size_t WaveFile::WaveFileReader::GetBitsPerSample()
{
	return ((header_.bitsPerSample_[1] << 8) + header_.bitsPerSample_[0]);
}

std::size_t WaveFile::WaveFileReader::GetByteRate()
{
	return (header_.byteRate_[3] << 24) + (header_.byteRate_[2] << 16) + (header_.byteRate_[1] << 8) + header_.byteRate_[0];
}

std::size_t WaveFile::WaveFileReader::GetBlockAlign()
{
	return (header_.blockAlign_[1] << 8) + header_.blockAlign_[0];
}

std::size_t WaveFile::WaveFileReader::GetSubChunk1Size()
{
	return (header_.subChunk1Size_[3] << 24) + (header_.subChunk1Size_[2] << 16) + (header_.subChunk1Size_[1] << 8) + header_.subChunk1Size_[0];
}

std::size_t WaveFile::WaveFileReader::GetSubChunk2Size() const
{
	return (header_.subChunk2Size_[3] << 24) + (header_.subChunk2Size_[2] << 16) + (header_.subChunk2Size_[1] << 8) + header_.subChunk2Size_[0];
}

std::size_t WaveFile::WaveFileReader::GetWaveHeaderSize()
{
	return waveHeaderSize_;
}

std::size_t WaveFile::WaveFileReader::GetChunkSize()
{
	return (header_.chunkSize_[3] << 24) + (header_.chunkSize_[2] << 16) + (header_.chunkSize_[1] << 8) + header_.chunkSize_[0];
}

std::vector<AudioData> WaveFile::WaveFileReader::GetAudioData()
{
	return GetAudioData(0, GetSampleCount());
}

std::vector<AudioData> WaveFile::WaveFileReader::GetAudioData(std::size_t samplesToRead)
{
	// First see if we're near the EOF and make sure we don't try and read beyong it as there may be 
	// metadata and etc there and we don't want to return that as if it were audio data.
	auto sampleCount{GetSampleCount()};
	auto samplePositionInFile{GetCurrentSamplePositionOfFilePointer()};
	if(samplePositionInFile > sampleCount)
	{
		Utilities::ThrowException("WaveFileReader internal file pointer position greater than total count of samples in file.", filename_, __FILE__, __LINE__);
	}

	std::size_t samplesRemainingInFile{GetSampleCount() - GetCurrentSamplePositionOfFilePointer()};
	if(samplesRemainingInFile < samplesToRead)
	{
		samplesToRead = samplesRemainingInFile;
	}

	std::size_t shortsToRead{samplesToRead * GetChannels()};
	std::vector<int16_t> data(shortsToRead);

	// Reinterpret cast here because ifstream's read wants a char* but we want to keep
	// our uint8_ts we read as unsigned chars (the WaveReeader::uint8_t type).
	inputFileStream_.read(reinterpret_cast<char*>(data.data()), shortsToRead * 2);
	if(!inputFileStream_.good())
	{
		Utilities::Exception(Utilities::Stringify("WaveFileReader::GetAudioData(std::size_t) failed to read audio data from wave file " + filename_));
	}

	if(GetChannels() == 1)
	{
		return std::vector<AudioData>{Signal::ConvertSigned16ToAudioData(data)};	
	}

	return Signal::ConvertInterleavedSigned16ToAudioData(data);	
}

std::vector<AudioData> WaveFile::WaveFileReader::GetAudioData(std::size_t samplesStartPosition, std::size_t samplesToRead)
{
	FilePointerSeekToSamplePosition(samplesStartPosition);
	return GetAudioData(samplesToRead);
}

const WaveFile::WaveFileHeader& WaveFile::WaveFileReader::GetHeader()
{
	return header_;
}

std::size_t WaveFile::WaveFileReader::GetSampleCount() const
{
	return sampleCount_;
}

std::size_t WaveFile::WaveFileReader::GetBytePositionInFileWhereAudioSamplesStart()
{
	// See wave file format defintitions for the how we calculate this value.  The wave header contains, starting at
	// the beginning of the file: Chunk ID (4 uint8_ts), ChunkSize (4 uint8_ts), Format (4 uint8_ts), SubChunk1ID (4 uint8_ts), 
	// SubChunk1Size (4 uint8_ts).

	const std::size_t uint8_tsThroughSubChunk1Size{20};

	// Then after SubChunk1(), but before the sample audio data starts, we have two 4 uint8_t values: the Subchunk2ID 
	// (4 uint8_ts), and SubChunk2Size (4 uint8_ts).
	const std::size_t uint8_tsBetweenSubChunk1AndAudioSamples{8};

	return (uint8_tsThroughSubChunk1Size + GetSubChunk1Size() + uint8_tsBetweenSubChunk1AndAudioSamples);
}

std::size_t WaveFile::WaveFileReader::GetByteCountOfSampleData()
{
	return GetSubChunk2Size();
}

std::size_t WaveFile::WaveFileReader::GetCurrentBytePositionOfFilePointer()
{
	return inputFileStream_.tellg();
}

std::size_t WaveFile::WaveFileReader::GetCurrentSamplePositionOfFilePointer()
{
	auto currentBytePosition{GetCurrentBytePositionOfFilePointer()};
	auto uint8_tPositionWhereAudioSamplesStart{GetBytePositionInFileWhereAudioSamplesStart()};

	if(uint8_tPositionWhereAudioSamplesStart > currentBytePosition)
	{
		Utilities::ThrowException("WaveFileReader internal file pointer position less than start of audio position.", filename_, __FILE__, __LINE__);
	}

	auto uint8_tPosition{currentBytePosition - uint8_tPositionWhereAudioSamplesStart};
	return (uint8_tPosition / GetChannels() / (GetBitsPerSample() / 8));
}

void WaveFile::WaveFileReader::FilePointerSeekToSamplePosition(std::size_t samplesStartPosition)
{
	// If the given position is beyond the end of samples, let's just set it to the EOF (or essentially EOF as there 
	// may be metadata or other 'stuff' after the audio sample data).
	if(samplesStartPosition > GetSampleCount())
	{
		samplesStartPosition = GetSampleCount();
	}

	std::size_t uint8_tPosition{GetBytePositionInFileWhereAudioSamplesStart() + (samplesStartPosition * GetChannels() * (GetBitsPerSample()/8))};

	inputFileStream_.seekg(uint8_tPosition, std::ios_base::beg);
}
