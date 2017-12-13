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

#include <ThreadSafeAudioFile/Writer.h>
#include <Utilities/Exception.h>
#include <AudioData/AudioData.h>
#include <algorithm>

ThreadSafeAudioFile::Writer::Writer(const std::string& filename, std::size_t channels, std::size_t sampleRate, std::size_t bitsPerSample) :
	waveFileWriter_{filename, channels, sampleRate, bitsPerSample}
{
	if(waveFileWriter_.GetChannels() == 2)
	{
		audioDataBuffers_.push_back(AudioData{});		
		audioDataBuffers_.push_back(AudioData{});		
	}
}

ThreadSafeAudioFile::Writer::~Writer() { }

void ThreadSafeAudioFile::Writer::WriteAudioStream(std::size_t streamID, const AudioData& audioData)
{
	std::lock_guard<std::mutex> lock{mutex_};

	if(waveFileWriter_.GetChannels() == 1)
	{
		waveFileWriter_.AppendAudioData(std::vector<AudioData>{audioData});
	}
	else // If not a mono file, it must be stereo (two channels)
	{
		auto oppositeStreamID{streamID ? 0 : 1};
		auto oppositeStreamBufferedAmount{audioDataBuffers_[oppositeStreamID].GetSize()};

		// So, we have the following possible cases:
		// 1) The opposite channel has zero samples buffered.  If this is the case, we should just 
		//    buffer all the given audioData.
		// 2) The opposite channel's buffer has greater than zero, but less than audioData.GetSize() 
		//    samples.  If this is the case, we should write oppositeAmount of samples and buffer the 
		//    remaining samples in audioData.
		// 3) The opposite channel's buffer has the same amount of samples that audioData has.  If so, 
		//    write the given samples and the opposite channel's buffer to the wave file.
		// 4) The opposite channel's buffer has more samples than the audioData has.  If so, write the  
		//    audioData.GetSize() amount of samples and leave the remaining samples in the opposite
		//    stream's buffer.

		if(oppositeStreamBufferedAmount == 0) // Case 1
		{
			audioDataBuffers_[streamID].Append(audioData);	
		}
		else  // Remaining cases
		{
			auto samplesToWrite{std::min(audioData.GetSize(), audioDataBuffers_[oppositeStreamID].GetSize())};

			std::vector<AudioData> dataToWrite(waveFileWriter_.GetChannels());
			dataToWrite[streamID].Append(audioData.Retrieve(samplesToWrite));
			dataToWrite[oppositeStreamID].Append(audioDataBuffers_[oppositeStreamID].RetrieveRemove(samplesToWrite));

			waveFileWriter_.AppendAudioData(dataToWrite);

			if(samplesToWrite < audioData.GetSize())  // Buffer whatever might remain of audioData
			{
				audioDataBuffers_[streamID].Append(audioData.Retrieve(samplesToWrite, audioData.GetSize() - samplesToWrite));
			}
		}

		maxBufferedSamples_ = std::max(std::max(audioDataBuffers_[0].GetSize(), audioDataBuffers_[1].GetSize()), maxBufferedSamples_);
	}
}

std::size_t ThreadSafeAudioFile::Writer::GetMaxBufferedSamples()
{
	return maxBufferedSamples_;
}
