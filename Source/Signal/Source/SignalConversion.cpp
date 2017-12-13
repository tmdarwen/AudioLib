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

#include <Signal/SignalConversion.h>
#include <algorithm>
#include <Utilities/Exception.h>
#include <Utilities/Stringify.h>

namespace Signal { namespace SignalConversion {

const int16_t MAX16{32767};
const int16_t MIN16{-32768};

constexpr double MAX16_FLOAT{32767.0};
constexpr double MIN16_FLOAT_REVERSE_SIGN{MIN16 * -1.0};

inline int16_t ConvertFloat64SampleToSigned16Sample(double sample)
{
	if(sample > 0.0)
	{
		if(sample < 1.0) 
		{
			return static_cast<int16_t>(sample * Signal::SignalConversion::MAX16_FLOAT + 0.5);
		}
		else
		{
			return Signal::SignalConversion::MAX16;
		}
	}
	else
	{
		if(sample < -1.0)
		{
			return MIN16;
		}
		else
		{
			return static_cast<int16_t>(sample * MIN16_FLOAT_REVERSE_SIGN - 0.5);
		}
	}
}

inline double ConvertSigned16SampleToFloat64(int16_t sample)
{
	if(sample == 0)
	{
		return 0.0;	
	}

	if(sample > 0)
	{
		if(sample < MAX16)
		{
			return static_cast<double>(sample) / Signal::SignalConversion::MAX16_FLOAT;
		}
		else
		{
			return 1.0;
		}
	}
	else
	{
		if(sample > MIN16)
		{
			return static_cast<double>(sample) / Signal::SignalConversion::MIN16_FLOAT_REVERSE_SIGN;
		}
		else
		{
			return -1.0;
		}
	}
}


}};

std::vector<int16_t> Signal::ConvertFloat64ToSigned16(const std::vector<double>& inputSignal)
{
	return Signal::ConvertFloat64ToSigned16(inputSignal, inputSignal.size());
}

std::vector<int16_t> Signal::ConvertFloat64ToSigned16(const std::vector<double>& inputSignal, std::size_t sampleCount)
{
	std::vector<int16_t> returnSignal;
	auto conversion{[&](double sample)
	{
		returnSignal.push_back(Signal::SignalConversion::ConvertFloat64SampleToSigned16Sample(sample));
	}};
		
	std::for_each(inputSignal.begin(), inputSignal.begin() + sampleCount, conversion);

	return returnSignal;
}

std::vector<double> Signal::ConvertSigned16ToFloat64(const std::vector<int16_t>& inputSignal)
{
	return Signal::ConvertSigned16ToFloat64(inputSignal, inputSignal.size());
}

std::vector<double> Signal::ConvertSigned16ToFloat64(const std::vector<int16_t>& inputSignal, std::size_t sampleCount)
{
	if(sampleCount > inputSignal.size())
	{
		Utilities::ThrowException("Requesting more sample than exist");
	}

	std::vector<double> returnSignal;
	double min16ReverseSign{-1.0 * Signal::SignalConversion::MIN16};

	auto conversion{[&](double sample) {
			if(sample > 0)
			{
				returnSignal.push_back(static_cast<double>(sample) /  Signal::SignalConversion::MAX16);
			}
			else
			{
				returnSignal.push_back(static_cast<double>(sample) / min16ReverseSign);
			}
	}};
		
	std::for_each(inputSignal.begin(), inputSignal.begin() + sampleCount, conversion);

	return returnSignal;
}

std::vector<int16_t> Signal::ConvertAudioDataToSigned16(const AudioData& audioData)
{
	std::vector<int16_t> returnSignal;
	for(std::size_t i{0}; i < audioData.GetSize(); ++i)
	{
		returnSignal.push_back(Signal::SignalConversion::ConvertFloat64SampleToSigned16Sample(audioData.GetData()[i]));
	}

	return returnSignal;
}

std::vector<int16_t> Signal::ConvertAudioDataToInterleavedSigned16(const AudioData& leftChannel, const AudioData& rightChannel)
{
	if(leftChannel.GetSize() != rightChannel.GetSize())
	{
		Utilities::ThrowException(Utilities::CreateString(" ", "Problem interleaving samples: Left channel has", leftChannel.GetSize(), "samples" \
						"and right channel has", rightChannel.GetSize(), "samples"));
	}

	std::vector<int16_t> returnSignal;
	for(std::size_t i{0}; i < leftChannel.GetSize(); ++i)
	{
		returnSignal.push_back(Signal::SignalConversion::ConvertFloat64SampleToSigned16Sample(leftChannel.GetData()[i]));
		returnSignal.push_back(Signal::SignalConversion::ConvertFloat64SampleToSigned16Sample(rightChannel.GetData()[i]));
	}

	return returnSignal;
}

AudioData Signal::ConvertSigned16ToAudioData(const std::vector<int16_t>& signal)
{
	AudioData audioData;

	std::vector<int16_t> returnSignal;
	for(std::size_t i{0}; i < signal.size(); ++i)
	{
		audioData.PushSample(Signal::SignalConversion::ConvertSigned16SampleToFloat64(signal[i]));
	}

	return audioData;
}

std::vector<AudioData> Signal::ConvertInterleavedSigned16ToAudioData(const std::vector<int16_t>& interleavedSigned16)
{
	AudioData audioDataLeft;
	AudioData audioDataRight;

	if(interleavedSigned16.size() % 2 != 0)
	{
		Utilities::ThrowException("Input given is not divisible by two and therefore cannot be interleaved");
	}

	std::vector<int16_t> returnSignal;
	for(std::size_t i{0}; i < interleavedSigned16.size();)
	{
		audioDataLeft.PushSample(Signal::SignalConversion::ConvertSigned16SampleToFloat64(interleavedSigned16[i++]));
		audioDataRight.PushSample(Signal::SignalConversion::ConvertSigned16SampleToFloat64(interleavedSigned16[i++]));
	}

	return std::vector<AudioData>{audioDataLeft, audioDataRight};
}
