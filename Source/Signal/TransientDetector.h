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

#include <AudioData/AudioData.h>

namespace Signal {

class TransientDetector
{
	public:
		// CTOR takes the sample rate of the audio it will process (e.g. 44100)
		TransientDetector(std::size_t sampleRate);
		virtual ~TransientDetector();

		// Return true if transients are found, false otherwise.
		// If transients are found, they will be in the transients variable
		bool FindTransients(const AudioData& audioInput, std::vector<std::size_t>& transients);

		// This is the ratio of valley-to-peak required to be considered a peak.  The default is 1.5.
		void SetValleyToPeakRatio(double ratio);
		double GetValleyToPeakRatio();

		// Setters for the level step settings
		void SetFirstLevelStep(double firstLevelStepMilliseconds);
		void SetSecondLevelStep(double secondLevelStepMilliseconds);
		void SetThirdLevelStep(double thirdLevelStepMilliseconds);

		// Getters for the level step settings
		double GetFirstLevelStep();
		double GetSecondLevelStep();
		double GetThirdLevelStep();

		// Clears internal data to prepare to perform transient detection on new audio
		void Reset();

		// The Transient Detector needs to "look ahead" just beyond the given amount of data in order to get a transient 
		// near the end of the given audio.  This returns to the user this "look ahead" amount.
		std::size_t GetLookAheadSampleCount();	

	private:
		struct PeakAndValley
		{
			PeakAndValley() : peak_{0}, valley_{0} { }
			PeakAndValley(std::size_t peak, std::size_t valley) : peak_{peak}, valley_{valley} { }
			std::size_t peak_;
			std::size_t valley_;
		};

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

		bool GetPeakAndValley(const AudioData& audioData, std::size_t stepSize, PeakAndValley& peakAndValley);

		std::size_t FindTransientSamplePosition(const PeakAndValley& firstLevelPeakAndValley);
	
		double GetMaxSample(const AudioData& audioData, std::size_t sampleCount);

		bool SampleIsPeak(double centerSample, double leftSample, double rightSample);

		bool SampleMeetsPeekRequirements(double peakSampleValue, double valleySampleValue);
};

}
