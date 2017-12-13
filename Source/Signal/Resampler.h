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

//! @file Resampler.h
//! @brief Implementation of an audio resampler.

#pragma once

#include <AudioData/AudioData.h>
#include <mutex>
#include <memory>

namespace Signal {

class LowPassFilter;

//! Implementation of a digital audio resampler using a windowed sinc filter.

//! A resampler allows for adjusting the sample rate of digital audio without unreasonably 
//! degrading the audio quality.

class Resampler
{
	public:
		//! Instatiate the resampler.
		//
		//! Example: An input sample rate of 44100Hz and a resample ratio of 0.5
		//! will result in an output sample rate of 22050Hz.
		Resampler(std::size_t inputSampleRate, double resampleRatio);

		virtual ~Resampler();

		//! Clears internal buffers and etc to allow for restarting processing fresh.
		void Reset();

		//! Submit audio data to be processed by the resampler.
		void SubmitAudioData(const AudioData& audioData);

		//! Retrieve output audio the resampler has processed, requesting a certain number of samples.
		AudioData GetAudioData(uint64_t samples);

		//! Returns the number of output samples currently available.
		std::size_t OutputSamplesAvailable();

		//! At the end of processing, this can be called to get any and all remaining output samples.
		AudioData FlushAudioData();

	private:			
		void ValidateSampleRates();
		void InstantiateLowPassFilter();
		void CalculateXSincCenterAdjustmentPerInputSample();
		void HandleNoSampleRateChange(const AudioData& audioData);
		void Process(const AudioData& audioData);
		AudioData LowPassFilterInput(const AudioData& audioData);
		void CheckForSincPositionWrapping();
		void DiscardInputNoLongerNeeded();

		std::size_t inputSampleRate_;
		double resampleRatio_;

		// This buffer holds input data waiting to be processed
		AudioData inputData_;

		// This buffer holds output data ready for the user to request
		AudioData outputData_;

		std::mutex mutex_;

		// We limit sample rate conversion to 1,000Hz-to-192,000Hz
		const std::size_t minimumSampleRate_{1000};
		const std::size_t maximumSampleRate_{192000};

		// See the document ResamplingUsingWindowedSincFilter.odg in SabbaticalNotes for more info on these constants.
		const std::size_t samplesPerSide_{19};                                             
		const std::size_t minimumSamplesNeededForProcessing_{(2 * samplesPerSide_) + 1}; // "+1" for the center index 
		                                                                                 // of the windowed sinc filter
		double xSincCenterAdjustmentPerInputSample_{0.0};
		double currentXSincPosition_{0.0};
		std::size_t inputSampleIndex_{samplesPerSide_};

		std::unique_ptr<Signal::LowPassFilter> lowPassFilter_;
};

}
