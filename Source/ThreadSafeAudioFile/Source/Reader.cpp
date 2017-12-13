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

#include <ThreadSafeAudioFile/Reader.h>
#include <Utilities/Exception.h>
#include <AudioData/AudioData.h>
#include <algorithm>

ThreadSafeAudioFile::Reader::Reader(const std::string& filename) : waveFileReader_{filename} { }

ThreadSafeAudioFile::Reader::~Reader() { }

std::size_t ThreadSafeAudioFile::Reader::GetChannels()
{
	return waveFileReader_.GetChannels();
}

std::size_t ThreadSafeAudioFile::Reader::GetSampleRate()
{
	return waveFileReader_.GetSampleRate();
}

std::size_t ThreadSafeAudioFile::Reader::GetBitsPerSample()
{
	return waveFileReader_.GetBitsPerSample();
}

std::size_t ThreadSafeAudioFile::Reader::GetSampleCount()
{
	return waveFileReader_.GetSampleCount();
}

AudioData ThreadSafeAudioFile::Reader::ReadAudioStream(std::size_t streamID, std::size_t sampleStartPosition, std::size_t samplesToRead)
{
	std::lock_guard<std::mutex> lock{mutex_};

	if(streamID > waveFileReader_.GetChannels())
	{
		Utilities::ThrowException(
			Utilities::Stringify("ReadAudioStream given stream ID greater than what exists.  Channels: ") + 
			Utilities::Stringify(waveFileReader_.GetChannels()) + 
			Utilities::Stringify(" streamID: ") + 
			Utilities::Stringify(streamID));	
	}

	// There is a lot of opportunity for making this more efficient, especially for stereo wave files.  Stereo wave files have their 
	// channels interleaved - left sample, right sample, repeat.  So when the waveFileReader does a read on a stereo file, right now 
	// we're just throwing away the entire other channel that it reads, when likely we're reading that other channeling in a different 
	// thread.  A buffering scheme could really improve this.

	return waveFileReader_.GetAudioData(sampleStartPosition, samplesToRead)[streamID];
}
