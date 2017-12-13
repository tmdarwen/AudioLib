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

//! @file PhaseVocoder.h
//! @brief Implementation of a phase vocoder.

#pragma once

#include <list>
#include <mutex>
#include <map>
#define _USE_MATH_DEFINES  // Seems some compilers need this so M_PI will be defined
#include <math.h>
#include <AudioData/AudioData.h>

namespace Signal {

class FrequencyDomain;
class PeakProfile;

//! Implementation of a phase vocoder.

//! A phase vocoder allows for stretching/compressing audio with respect to time.  Wikipedia has a pretty good explanation 
//! of how a phase vocoder works.

class PhaseVocoder
{
	public:
		//! Instatiate the phase vocoder.
		// 
		//! Params:
		//! 1) The sample rate of the audio it will process (e.g. 44100) 
		//! 2) The total length in samples of the input we'll be stretching
		//! 3) The stretch factor which is a ratio of the input (e.g. 1.0 = no change, 0.8 = 20% speedup, 1.2 = 20% slowdown)
		PhaseVocoder(std::size_t sampleRate, std::size_t inputLength, double stretchFactor);
		virtual ~PhaseVocoder();

		//! Clears internal buffers and etc to allow for restarting processing fresh.
		void Reset();

		//! Submit audio data to be processed by the phase vocoder.
		void SubmitAudioData(const AudioData& audioData);

		//! Retrieve output audio the Phase Vocoder has processed, requesting a certain number of samples.
		AudioData GetAudioData(uint64_t samples);

		//! Returns the number of output samples currently available.
		std::size_t OutputSamplesAvailable();

		//! At the end of processing, this can be called to get any and all remaining output samples.
		AudioData FlushAudioData();

		//! Returns the stretch factor given at construction.
		double GetStretchFactor();

	private:
		bool CheckForEdgeCases();
		bool CheckForNoStretchEdgeCase();
		bool CheckForShortInputEdgeCases();

		void DoPrecalculations();

		void HandleNoStretchInput(const AudioData& audioData);
		AudioData HandleShortInputCompress();

		void ProcessBuffer();

		void HandleFirstWindow(Signal::FrequencyDomain& frequencyDomain);
		void CreateSynthesizedOutputWindow(Signal::FrequencyDomain& frequencyDomain, std::size_t advancement);

		std::map<std::size_t, double> GetPeakFrequencies(const AudioData& timeDomainSignal, Signal::PeakProfile& peakProfile);

		double CalculateNewPhaseWrapped(std::size_t currentBin, double currentWrappedPhase, double peakFrequency, std::size_t advancement);

		double ConvertUnwrappedPhaseToWrappedPhase(double unwrappedPhase);

		void OverlapAndAddForOutput(AudioData& newSythesizedWindow);
		AudioData MixAtBestCorrelation(const AudioData& transientBuffer, const AudioData& stretchBuffer);

		bool noStretch_{false};
		bool shortInputCompress_{false};

		std::size_t sampleRate_;
		std::size_t inputLength_;
		double stretchFactor_;
		std::size_t minimumOutputSamplesNecessary_{0};

		double transientCutoff_{0.0};

		double sampleAdvancement_{0.0};
		double sampleAdvancementRemainder_{0.0};

		std::size_t inputSamplesProcessed_{0};

		// Holds a count of the total number of windows processed
		std::size_t windowsProcessed_{0};
		std::size_t totalOutputSamplesCreated_{0};

		// This buffer holds input data waiting to be processed
		AudioData inputData_;

		// Remember that we create the output through a 4x overlap-and-add process.  This requires 
		// us to have the three previous synthesized buffers in addition to the latest one.
		std::list<AudioData> windowsInUse_;

		// This buffer holds output data ready for the user to request
		AudioData outputData_;

		// Holds any transient audio that needs to be mixed into the output
		AudioData transientSamples_;  

		std::vector<double> previousWrappedPhases_;
		std::vector<double> previousExtrapolatedUnwrappedPhases_;

		std::mutex mutex_;

		static const uint32_t FFT_SIZE{4096};
		static const uint32_t QUARTER_FFT_SIZE{FFT_SIZE/4};
		static constexpr double TWO_PI_RADIANS{2.0 * M_PI};
		static const uint32_t OVERLAP_FACTOR{4};

		static constexpr double optimalTransientCutoff_{QUARTER_FFT_SIZE * 3.0};

		// I came up with this amp factors just through trial and error on a signal with contant amplitude
		static constexpr double SYNTHEIZED_OVERLAP_AMP_FACTOR{0.8024};
};

}