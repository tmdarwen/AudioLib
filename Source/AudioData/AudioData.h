#pragma once

#include <vector>
#include <cstdint>

// Essentially just a FIFO buffer
class AudioData
{
	public:
		AudioData();
		AudioData(const std::vector<double>& data);
		AudioData(const double* data, std::size_t samples);
		AudioData(const AudioData&);
		AudioData(AudioData&&);
		virtual ~AudioData();

		void operator=(AudioData&);
		AudioData& operator=(AudioData&&);

		void AddSilence(uint64_t sampleCount);

		std::size_t GetSize() const;

		// Returns a read only reference to the data
		const std::vector<double>& GetData() const;

		// Returns a reference (not a copy) to the actual encapsulated data buffer for direct write access
		std::vector<double>& GetDataWriteAccess();

		void PushSample(double sample);
		void PushBuffer(double* buffer, std::size_t size);
		void PushBuffer(const std::vector<double>& buffer);
		void PushBuffer(const std::vector<double>& buffer, std::size_t size);

		// Appends the given AudioData samples to the data_ buffer.
		void Append(const AudioData& audioData);

		// Retrieves the given number of samples from the beginning of the FIFO buffer 
		// returning them in the AudioData return value
		AudioData Retrieve(uint64_t samples) const;

		// Retrieves the given number of samples from the beginning of the FIFO buffer 
		// returning them in the AudioData return value and removing them from data_
		AudioData RetrieveRemove(uint64_t samples);

		// Retrieves the given number of samples starting at the given start position.
		// Does not remove the samples from the "this" (i.e. source) AudioData.
		AudioData Retrieve(uint64_t startPosition, uint64_t samples) const;

		void Truncate(std::size_t newSize);

		void MixInSamples(const AudioData& audioData);
		void MixInSamples(const double* buffer, std::size_t samples);

		// Moves the last given number of samples into the targetAudioData
		void MoveLastSamples(std::size_t samples, AudioData& targetAudioData);

		void RemoveFrontSamples(std::size_t samples);

		void Clear();

		void LinearCrossfade(AudioData& audioData);

		void Amplify(double ratio);
		void Amplify(double beginRatio, double endRatio);

		//void LinearAmplify(double startRatio, double endRatio);

	private:
		std::vector<double> data_;

};

// Linearly crossfades param1's audio with param2's audio returning the results.
// Param1's audio is at 100% at the start and 0% at the end.  Param2's audio is 
// at 0% at the start and 100% at the end.  If param1's audio is length doesn't 
// match param2's length we start mixing at sample number zero of both audio 
// inputs and crossfade over the shortest duration.
AudioData LinearCrossfade(const AudioData&, const AudioData&);
