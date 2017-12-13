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

//! @file AudioData.h
//! @brief A class allowing for easy transportation and access of digital audio data. 

#pragma once

#include <vector>
#include <cstdint>

//! A class allowing for easy transportation and access of digital audio data. 
//
//! This class simply wraps a std::vector allowing various typical audio buffer 
//! operations.  Audio data consist of an arbitrary number of "samples".  Samples
//! are 64 bit floating point numbers that range from 1.0 (peak positive sample) 
//! to -1.0 (peak negative sample).

class AudioData
{
	public:
		//! Constructs an audio buffer with size of zero.
		AudioData();

		//! Constructs an audio buffer with the given audio data.
		AudioData(const std::vector<double>& data);

		//! Constructs an audio buffer with the given audio data.
		AudioData(const double* data, std::size_t samples);

		//! Constructs an audio buffer with the given audio data.
		AudioData(const AudioData&);

		//! Constructs an audio buffer with the given audio data.
		AudioData(AudioData&&);

		//! Virtual destructor in case of being subclassed.
		virtual ~AudioData();

		//! Assignment operator using the copy-and-swap idiom.
		AudioData& operator=(AudioData);

		//! Inserts the given samples of silence (i.e. samples with value of 0.0).
		void AddSilence(uint64_t sampleCount);

		//! Returns the number of samples currently contained in the buffer.
		std::size_t GetSize() const;

		//! Returns a read only reference to the audio data.
		const std::vector<double>& GetData() const;

		//! Returns a reference (not a copy) to the actual encapsulated data buffer for direct write access
		std::vector<double>& GetDataWriteAccess();

		//! Pushes the given sample onto the FIFO buffer.
		void PushSample(double sample);

		//! Pushes the given samples onto the FIFO buffer.
		void PushBuffer(double* buffer, std::size_t size);

		//! Pushes the given samples onto the FIFO buffer.
		void PushBuffer(const std::vector<double>& buffer);

		//! Pushes the given samples onto the FIFO buffer.
		//
		//! Note that if the "buffer" argument contains more than the "size" argument samples, only the first 
		//! "size" samples of the "buffer" will be pushed.
		void PushBuffer(const std::vector<double>& buffer, std::size_t size);

		//! Appends the given AudioData samples to the buffer.
		void Append(const AudioData& audioData);

		//! Retrieves the given number of samples from the beginning of the FIFO buffer.
		//
		//! The retrived samples are returned in an AudioData object.  Note that the retrieved samples are 
		//! not removed from the object upon which Retrieve is called.
		AudioData Retrieve(uint64_t samples) const;

		//! Retrieves the given number of samples starting at the given start position.
		//
		//! The retrived samples are returned in an AudioData object.  Note that the retrieved samples are 
		//! not removed from the object upon which Retrieve is called.
		AudioData Retrieve(uint64_t startPosition, uint64_t samples) const;

		//! Retrieves (and removes) the given number of samples
		//
		//! The samples retrieved are returned in an AudioData object.  These same samples are removed from 
		//! the object upon which Retrieve is called.
		AudioData RetrieveRemove(uint64_t samples);

		//! Truncates the size of the AudioData to the newSize number of samples.
		//	
		//! Any of the current samples in the AudioData that are in-excess of given newsize will be removed from 
		//! the end of the buffer.
		void Truncate(std::size_t newSize);

		//! Mixes the given audio data with the object's current audio data.
		void MixInSamples(const AudioData& audioData);

		//! Mixes the given audio data with the object's current audio data.
		void MixInSamples(const double* buffer, std::size_t samples);

		//! Moves the last given number of samples into targetAudioData.
		//
		//! By "last given number of samples" we mean the samples at the very end of the buffer are moved into 
		//! the targetAudioData.  These samples are then removed from the source buffer upon which this method 
		//! was called.
		void MoveLastSamples(std::size_t samples, AudioData& targetAudioData);

		//! Removes the given number of samples from the front of the FIFO buffer.
		void RemoveFrontSamples(std::size_t samples);

		//! Completely empties the FIFO buffer.
		void Clear();

		//! Crossfades the given audio with the object's current audio data.
		//
		//! This is a linear crossfade with the level of the internal audio data starting at 100% and 
		//! ending at 0%, and the level of the given audio starting at a 0% level and ending at 100%.
		void LinearCrossfade(AudioData& audioData);

		//! Amplifies the audio by the given ratio.
		//
		//! By example, a ratio of 1.0 is no change.  A ratio of 0.5 is reducing the volume (linearly) 
		//! by 50% and a ratio of 2.0 is doubling the volume level.
		void Amplify(double ratio);

		//! Linearly amplifies the volume with linear change rate.
		//
		//! By example, a beginRatio of 0.75 and endRatio of 1.25 would reduce the audio at the very 
		//! beginning by 25%, would be unchanged at the midpoint and would be amplified by 25% at the 
		//! very end.
		void Amplify(double beginRatio, double endRatio);

	private:
		std::vector<double> data_;

};

//! Linearly crossfades param1's audio with param2's audio returning the results.
//
//! Param1's audio is at 100% at the start and 0% at the end.  Param2's audio is 
//! at 0% at the start and 100% at the end.  If param1's audio is length doesn't 
//! match param2's length we start mixing at sample number zero of both audio 
//! inputs and crossfade over the shortest duration.
AudioData LinearCrossfade(const AudioData&, const AudioData&);
