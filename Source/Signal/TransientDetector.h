/*
 * AudioLib
 *
 * Copyright (c) 2017-2018 Terence M. Darwen - tmdarwen.com
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

//! @file TransientDetector.h
//! @brief Implementation of a transient detector.

#pragma once

#include <AudioData/AudioData.h>
#include <Signal/TransientPeakAndValley.h>

namespace Signal {

//! Implementation of a transient detector.

//! The transient detector, detects transients in audio.  An audio transient is usually a high amplitude, 
//! short-duration sound at the beginning of a waveform.
//! See https://github.com/tmdarwen/AudioLib/blob/master/Documentation/TransientDetection.md

class TransientDetector
{
	public:
		//! Instantiates the transient detector taking the sample rate of the audio it will process (e.g. 44100).
		TransientDetector(std::size_t sampleRate);
		virtual ~TransientDetector();

		//! Finds the sample positions of transients in the given audio.
		//
		//! Return true if transients are found, false otherwise.
		//! If transients are found, they will be in the transients variable
		bool FindTransients(const AudioData& audioInput, std::vector<std::size_t>& transients);

		//! The ratio of valley-to-peak growth for audio to be considered a peak.  The default is 1.5.
		void SetValleyToPeakRatio(double ratio);

		//! Returns the current valley-to-peak ratio.
		double GetValleyToPeakRatio();

		//! Set the minimum audio level of a peak sample to actually qualify as a peak. The default is 0.1.
		void SetMinimumPeakLevel(double minPeakLevel);

		//! Get the minimum audio level of a peak sample to actually qualify as a peak.
		double GetMinimumPeakLevel();

		//! Set first level step setting.  Default is 11.6 milliseconds.
		void SetFirstLevelStep(double firstLevelStepMilliseconds);

		//! Set second level step setting.  Default is 5.8 milliseconds.
		void SetSecondLevelStep(double secondLevelStepMilliseconds);

		//! Set third level step setting.  Default is 0.73 milliseconds.
		void SetThirdLevelStep(double thirdLevelStepMilliseconds);

		//! Set first level step setting in samples.  Default is 512 samples for 44.1 KHz signal.
		void SetFirstLevelStepInSamples(std::size_t samples);

		//! Set second level step setting in samples.  Default is 256 samples for 44.1 KHz signal.
		void SetSecondLevelStepInSamples(std::size_t samples);

		//! Set third level step setting in samples.  Default is 32 samples for 44.1 KHz signal.
		void SetThirdLevelStepInSamples(std::size_t samples);

		//! Get the current first level step setting in milliseconds
		double GetFirstLevelStep();

		//! Get the current second level step setting in milliseconds
		double GetSecondLevelStep();

		//! Get the current third level step setting in milliseconds
		double GetThirdLevelStep();

		//! Get the current first level step size in sample units.
		std::size_t GetFirstLevelStepInSamples();

		//! Get the current second level step size in sample units.
		std::size_t GetSecondLevelStepInSamples();

		//! Get the current third level step size in sample units.
		std::size_t GetThirdLevelStepInSamples();

		//! Clears internal data to prepare to perform transient detection on new audio.
		void Reset();

		//! Gets the actual first step values for the given audio
		std::vector<double> GetFirstStepValues(const AudioData& audioInput);

		//! Get the number of "look ahead" samples required.
		//
		//! The Transient Detector needs to "look ahead" just beyond the given amount of data in order to get a transient 
		//! near the end of the given audio.  This returns to the user this "look ahead" amount.
		std::size_t GetLookAheadSampleCount();	

	private:
		std::size_t sampleRate_;
		std::vector<TransientPeakAndValley> firstLevel_;
		std::vector<TransientPeakAndValley> secondLevel_;
		std::vector<TransientPeakAndValley> thirdLevel_;

		double firstLevelStepMilliseconds_{11.60998};  // 512 samples for 44.1KHz sample rate
		double secondLevelStepMilliseconds_{5.80499};  // 256 samples for 44.1KHz sample rate
		double thirdLevelStepMilliseconds_{0.725623};  // 32 samples for 44.1KHz sample rate

		std::size_t firstLevelStepSize_;
		std::size_t secondLevelStepSize_;
		std::size_t thirdLevelStepSize_;

		double minValleyToPeakGrowthRatio_{1.5};  // The amount a peak's growth must be over the preceding valley
		double minPeakLevel_{0.1};  // The minimum amplitude value to be considered a peak

		AudioData audioDataInput_;

		bool transientsFound_{false};  // False until a transient is found
		std::size_t lastTransientValue_{0};  // Stores sample position of the last transient output

		std::size_t inputSamplesProcessed_{0};  // Keeps track of how many input samples the transient detector has analyzed

		// The amount of audio to preserve from the past when detecting peak/valleys
		std::size_t secondsOfPastAudioToRetain_{1};
		std::size_t samplesOfPastAudioToRetain_;

		void CheckForOldAudio();

		bool CheckForEnoughAudioToProcess();

		bool CheckForAllSilence();

		// Return true if transients are found, false otherwise.
		// If transients are found, they will be in the transients variable
		bool FindTransients(std::vector<std::size_t>& transients);

		std::size_t FindFirstTransient();

		bool GetPeakAndValley(const AudioData& audioData, std::size_t stepSize, TransientPeakAndValley& peakAndValley);

		std::size_t FindTransientSamplePosition(const TransientPeakAndValley& firstLevelPeakAndValley);
	
		double GetMaxSample(const AudioData& audioData, std::size_t sampleCount);

		bool SampleIsPeak(double centerSample, double leftSample, double rightSample);

		bool SampleMeetsPeekRequirements(double peakSampleValue, double valleySampleValue);
};

}
