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

#include <gtest/gtest.h>
#include <AudioData/AudioData.h>
#include <Utilities/Exception.h>

// The fixture for testing class Foo.
class AudioDataTest : public testing::Test
{
	public:
		AudioData audioData;

		AudioDataTest()
		{
			audioData.PushSample(0.5);
			audioData.PushSample(0.6);
			audioData.PushSample(0.7);
			audioData.PushSample(0.8);
		}
};

TEST_F(AudioDataTest, TestCopyConstruction)
{
	AudioData copy{audioData};

	{
		auto data = audioData.GetData();
		EXPECT_EQ(4, data.size());
		EXPECT_EQ(0.5, data[0]);
		EXPECT_EQ(0.6, data[1]);
		EXPECT_EQ(0.7, data[2]);
		EXPECT_EQ(0.8, data[3]);
	}

	{
		auto data = copy.GetData();
		EXPECT_EQ(4, data.size());
		EXPECT_EQ(0.5, data[0]);
		EXPECT_EQ(0.6, data[1]);
		EXPECT_EQ(0.7, data[2]);
		EXPECT_EQ(0.8, data[3]);
	}
}

TEST_F(AudioDataTest, TestMoveConstruction)
{
	AudioData moved{std::move(audioData)};

	{
		auto data = audioData.GetData();
		EXPECT_EQ(0, data.size());
	}

	{
		auto data = moved.GetData();
		EXPECT_EQ(4, data.size());
		EXPECT_EQ(0.5, data[0]);
		EXPECT_EQ(0.6, data[1]);
		EXPECT_EQ(0.7, data[2]);
		EXPECT_EQ(0.8, data[3]);
	}
}

TEST_F(AudioDataTest, TestAssignment)
{
	AudioData assigned;
	assigned = audioData;

	{
		auto data = audioData.GetData();
		EXPECT_EQ(4, data.size());
		EXPECT_EQ(0.5, data[0]);
		EXPECT_EQ(0.6, data[1]);
		EXPECT_EQ(0.7, data[2]);
		EXPECT_EQ(0.8, data[3]);
	}

	{
		auto data = assigned.GetData();
		EXPECT_EQ(4, data.size());
		EXPECT_EQ(0.5, data[0]);
		EXPECT_EQ(0.6, data[1]);
		EXPECT_EQ(0.7, data[2]);
		EXPECT_EQ(0.8, data[3]);
	}
}

TEST_F(AudioDataTest, TestEmptyBuffer)
{
	AudioData emptyAudioData;
	EXPECT_EQ(0, emptyAudioData.GetSize());

	auto data = emptyAudioData.GetData();
	EXPECT_EQ(0, data.size());
}

TEST_F(AudioDataTest, TestAddingData)
{
	EXPECT_EQ(4, audioData.GetSize());

	audioData.PushSample(0.9);
	EXPECT_EQ(5, audioData.GetSize());

	auto data = audioData.GetData();
	EXPECT_EQ(5, data.size());
}

TEST_F(AudioDataTest, TestRetrievingData)
{
	auto data = audioData.GetData();
	EXPECT_EQ(4, data.size());
	EXPECT_EQ(0.5, data[0]);
	EXPECT_EQ(0.6, data[1]);
	EXPECT_EQ(0.7, data[2]);
	EXPECT_EQ(0.8, data[3]);

	EXPECT_EQ(4, audioData.GetSize());
}

TEST_F(AudioDataTest, TestModifyingData)
{
	std::vector<double>& dataWriteAccess{audioData.GetDataWriteAccess()};

	dataWriteAccess[2] = 1.0;

	auto data = audioData.GetData();

	EXPECT_EQ(4, data.size());
	EXPECT_EQ(0.5, data[0]);
	EXPECT_EQ(0.6, data[1]);
	EXPECT_EQ(1.0, data[2]);
	EXPECT_EQ(0.8, data[3]);

	EXPECT_EQ(4, audioData.GetSize());
}

TEST_F(AudioDataTest, TestClearingBuffer)
{
	audioData.Clear();
	EXPECT_EQ(0, audioData.GetSize());

	auto data = audioData.GetData();
	EXPECT_EQ(0, data.size());
}

TEST_F(AudioDataTest, TestLinearCrossfade)
{
	AudioData audioData2;
	audioData2.PushSample(0.5);
	audioData2.PushSample(0.4);
	audioData2.PushSample(0.3);
	audioData2.PushSample(0.2);

	audioData.LinearCrossfade(audioData2);
	EXPECT_EQ(4, audioData.GetSize());

	auto data = audioData.GetData();
	EXPECT_EQ(0.5, data[0]);
	EXPECT_NEAR(0.5333, data[1], 0.0001);
	EXPECT_NEAR(0.4333, data[2], 0.0001);
	EXPECT_EQ(0.2, data[3]);
}

TEST_F(AudioDataTest, TestAppend)
{
	AudioData newAudioData;
	newAudioData.PushSample(0.9);
	newAudioData.PushSample(1.0);
	newAudioData.PushSample(1.1);
	newAudioData.PushSample(1.2);

	audioData.Append(newAudioData);

	auto data = audioData.GetData();

	EXPECT_EQ(8, data.size());

	EXPECT_EQ(0.5, data[0]);
	EXPECT_EQ(0.6, data[1]);
	EXPECT_EQ(0.7, data[2]);
	EXPECT_EQ(0.8, data[3]);
	EXPECT_EQ(0.9, data[4]);
	EXPECT_EQ(1.0, data[5]);
	EXPECT_EQ(1.1, data[6]);
	EXPECT_EQ(1.2, data[7]);
}

TEST_F(AudioDataTest, TestRetrieve)
{
	auto newAudioData = audioData.Retrieve(2);
	auto oldData = audioData.GetData();
	auto newData = newAudioData.GetData();

	EXPECT_EQ(4, oldData.size());
	EXPECT_EQ(2, newData.size());

	EXPECT_EQ(0.5, newData[0]);
	EXPECT_EQ(0.6, newData[1]);

	EXPECT_EQ(0.5, oldData[0]);
	EXPECT_EQ(0.6, oldData[1]);
	EXPECT_EQ(0.7, oldData[2]);
	EXPECT_EQ(0.8, oldData[3]);
}

TEST_F(AudioDataTest, TestRetrieveLastTwo)
{
	auto newAudioData = audioData.Retrieve(2, 2);  // Retrieve 2 sample starting at index 2
	auto oldData = audioData.GetData();
	auto newData = newAudioData.GetData();

	EXPECT_EQ(4, oldData.size());
	//EXPECT_EQ(2, newData.size());

	EXPECT_EQ(0.7, newData[0]);
	EXPECT_EQ(0.8, newData[1]);

/*
	EXPECT_EQ(0.5, oldData[0]);
	EXPECT_EQ(0.6, oldData[1]);
	EXPECT_EQ(0.7, oldData[2]);
	EXPECT_EQ(0.8, oldData[3]);
 */
}


TEST_F(AudioDataTest, TestRetrieveError)
{
	// Attempting to retrieve more samples than exist in the AudioData buffer should throw an exception
	ASSERT_THROW(audioData.Retrieve(5), Utilities::Exception);
}

TEST_F(AudioDataTest, TestRetrieveRemove)
{
	auto newAudioData = audioData.RetrieveRemove(2);
	auto oldData = audioData.GetData();
	auto newData = newAudioData.GetData();

	EXPECT_EQ(2, oldData.size());
	EXPECT_EQ(2, newData.size());

	EXPECT_EQ(0.5, newData[0]);
	EXPECT_EQ(0.6, newData[1]);

	EXPECT_EQ(0.7, oldData[0]);
	EXPECT_EQ(0.8, oldData[1]);
}

TEST_F(AudioDataTest, TestAmplify)
{
	audioData.Amplify(0.5);
	auto data = audioData.GetData();

	EXPECT_EQ(4, data.size());

	EXPECT_EQ(0.25, data[0]);
	EXPECT_EQ(0.3, data[1]);
	EXPECT_EQ(0.35, data[2]);
	EXPECT_EQ(0.4, data[3]);
}

TEST_F(AudioDataTest, TestAmplify2)
{
	AudioData audioData;
	for(std::size_t i = 0; i < 10; ++i)
	{
		audioData.PushSample(1.0);
	}

	audioData.Amplify(0.0, 1.0);
	auto data = audioData.GetData();

	EXPECT_EQ(10, data.size());

	EXPECT_NEAR(0.0, data[0], 0.0001);
	EXPECT_NEAR(0.1111, data[1], 0.0001);
	EXPECT_NEAR(0.2222, data[2], 0.0001);
	EXPECT_NEAR(0.3333, data[3], 0.0001);
	EXPECT_NEAR(0.4444, data[4], 0.0001);
	EXPECT_NEAR(0.5555, data[5], 0.0001);
	EXPECT_NEAR(0.6666, data[6], 0.0001);
	EXPECT_NEAR(0.7777, data[7], 0.0001);
	EXPECT_NEAR(0.8888, data[8], 0.0001);
	EXPECT_NEAR(1.0, data[9], 0.0001);
}

TEST_F(AudioDataTest, TestAmplify3)
{
	AudioData audioData;
	for(std::size_t i = 0; i < 10; ++i)
	{
		audioData.PushSample(1.0);
	}

	audioData.Amplify(1.0, 0.0);
	auto data = audioData.GetData();

	EXPECT_EQ(10, data.size());

	EXPECT_NEAR(1.0, data[0], 0.0001);
	EXPECT_NEAR(0.8888, data[1], 0.0001);
	EXPECT_NEAR(0.7777, data[2], 0.0001);
	EXPECT_NEAR(0.6666, data[3], 0.0001);
	EXPECT_NEAR(0.5555, data[4], 0.0001);
	EXPECT_NEAR(0.4444, data[5], 0.0001);
	EXPECT_NEAR(0.3333, data[6], 0.0001);
	EXPECT_NEAR(0.2222, data[7], 0.0001);
	EXPECT_NEAR(0.1111, data[8], 0.0001);
	EXPECT_NEAR(0.0, data[9], 0.0001);
}

TEST_F(AudioDataTest, TestAmplify4)
{
	AudioData audioData;
	for(std::size_t i = 0; i < 10; ++i)
	{
		audioData.PushSample(0.5);
	}

	audioData.Amplify(0.25, 1.50);
	auto data = audioData.GetData();

	EXPECT_EQ(10, data.size());

	EXPECT_NEAR(0.125, data[0], 0.0001);
	EXPECT_NEAR(0.1944, data[1], 0.0001);
	EXPECT_NEAR(0.2639, data[2], 0.0001);
	EXPECT_NEAR(0.3333, data[3], 0.0001);
	EXPECT_NEAR(0.4028, data[4], 0.0001);
	EXPECT_NEAR(0.4722, data[5], 0.0001);
	EXPECT_NEAR(0.5417, data[6], 0.0001);
	EXPECT_NEAR(0.6111, data[7], 0.0001);
	EXPECT_NEAR(0.6806, data[8], 0.0001);
	EXPECT_NEAR(0.75, data[9], 0.0001);
}

TEST_F(AudioDataTest, TestAmplify5)
{
	AudioData audioData;
	for(std::size_t i = 0; i < 10; ++i)
	{
		audioData.PushSample(1.0);
	}

	audioData.Amplify(0.75, 0.25);
	auto data = audioData.GetData();

	EXPECT_EQ(10, data.size());

	EXPECT_NEAR(0.75, data[0], 0.0001);
	EXPECT_NEAR(0.6944, data[1], 0.0001);
	EXPECT_NEAR(0.6389, data[2], 0.0001);
	EXPECT_NEAR(0.5833, data[3], 0.0001);
	EXPECT_NEAR(0.5278, data[4], 0.0001);
	EXPECT_NEAR(0.4722, data[5], 0.0001);
	EXPECT_NEAR(0.4167, data[6], 0.0001);
	EXPECT_NEAR(0.3611, data[7], 0.0001);
	EXPECT_NEAR(0.3056, data[8], 0.0001);
	EXPECT_NEAR(0.25, data[9], 0.0001);
}

// Scenario1 is where both inputs are the same length
TEST_F(AudioDataTest, TestLinearCrossfadeHelperScenario1)
{
	AudioData audioData1;
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);

	AudioData audioData2;
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);

	auto results{LinearCrossfade(audioData1, audioData2)};
	EXPECT_EQ(6, results.GetSize());

	auto data{results.GetData()};
	EXPECT_EQ(1.0, data[0]);
	EXPECT_EQ(1.0, data[1]);
	EXPECT_EQ(1.0, data[2]);
	EXPECT_EQ(1.0, data[3]);
	EXPECT_EQ(1.0, data[4]);
	EXPECT_EQ(1.0, data[5]);
}

// Scenario2 is where the first buffer is longer than the second
TEST_F(AudioDataTest, TestLinearCrossfadeHelperScenario2)
{
	AudioData audioData1;
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(0.75);  // Just make the extra samples some other 
	audioData1.PushSample(0.75);  // value than 1.0 and make sure it doesn't change.

	AudioData audioData2;
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);

	auto results{LinearCrossfade(audioData1, audioData2)};
	EXPECT_EQ(6, results.GetSize());

	auto data{results.GetData()};
	EXPECT_EQ(1.0, data[0]);
	EXPECT_EQ(1.0, data[1]);
	EXPECT_EQ(1.0, data[2]);
	EXPECT_EQ(1.0, data[3]);
	EXPECT_EQ(0.75, data[4]);
	EXPECT_EQ(0.75, data[5]);
}

// Scenario3 is where the second buffer is longer than the first
TEST_F(AudioDataTest, TestLinearCrossfadeHelperScenario3)
{
	AudioData audioData1;
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);
	audioData1.PushSample(1.0);

	AudioData audioData2;
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(1.0);
	audioData2.PushSample(0.75);  // Just make the extra samples some other 
	audioData2.PushSample(0.75);  // value than 1.0 and make sure it doesn't change.

	auto results{LinearCrossfade(audioData1, audioData2)};
	EXPECT_EQ(6, results.GetSize());

	auto data{results.GetData()};
	EXPECT_EQ(1.0, data[0]);
	EXPECT_EQ(1.0, data[1]);
	EXPECT_EQ(1.0, data[2]);
	EXPECT_EQ(1.0, data[3]);
	EXPECT_EQ(0.75, data[4]);
	EXPECT_EQ(0.75, data[5]);
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

