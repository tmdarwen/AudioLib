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
#include <Signal/TransientPeakAndValley.h>
#include <algorithm>

TEST(TransientPeakAndValley, InstantiateTest)
{
	Signal::TransientPeakAndValley transientPeakAndValley(100, 256);
	EXPECT_EQ(100, transientPeakAndValley.GetStartSamplePosition());
	EXPECT_EQ(256, transientPeakAndValley.GetStepSize());
	EXPECT_EQ(0, transientPeakAndValley.GetPeakSamplePosition());
	EXPECT_EQ(0, transientPeakAndValley.GetValleySamplePosition());
	EXPECT_EQ(0, transientPeakAndValley.GetPeakPoint());
	EXPECT_EQ(0, transientPeakAndValley.GetValleyPoint());
	EXPECT_EQ(0, transientPeakAndValley.GetPlottedPoints().size());
}

/*
TEST(TransientPeakAndValley, TestBasicValues)
{
	Signal::TransientPeakAndValley transientPeakAndValley(100, 256);

	std::vector<double> hypotheticalPoints{0.1, 0.5, 0.2, 0.3, 0.7, 0.4, 0.6, 0.8, 0.7};
	std::for_each(hypotheticalPoints.begin(), hypotheticalPoints.end(), [&](double point) { transientPeakAndValley.PushPlottedPoint(point);  });

	transientPeakAndValley.SetPeakSamplePosition(768);
	transientPeakAndValley.SetValleySamplePosition(512);

	auto plottedPoints{transientPeakAndValley.GetPlottedPoints()};

	EXPECT_EQ(100, transientPeakAndValley.GetStartSamplePosition());
	EXPECT_EQ(256, transientPeakAndValley.GetStepSize());
	EXPECT_EQ(7, transientPeakAndValley.GetPeakPoint());
	EXPECT_EQ(5, transientPeakAndValley.GetValleyPoint());
	EXPECT_EQ(hypotheticalPoints.size(), plottedPoints.size());
	EXPECT_TRUE(std::equal(hypotheticalPoints.begin(), hypotheticalPoints.end(), plottedPoints.begin(), plottedPoints.end()));
}

TEST(TransientPeakAndValley, TestReset)
{
	Signal::TransientPeakAndValley transientPeakAndValley(100, 256);

	std::vector<double> hypotheticalPoints{0.1, 0.5, 0.2, 0.3, 0.7, 0.4, 0.6, 0.8, 0.7};
	std::for_each(hypotheticalPoints.begin(), hypotheticalPoints.end(), [&](double point) { transientPeakAndValley.PushPlottedPoint(point);  });

	transientPeakAndValley.SetPeakSamplePosition(7);
	transientPeakAndValley.SetValleySamplePosition(5);

	transientPeakAndValley.Reset(200, 400);

	EXPECT_EQ(200, transientPeakAndValley.GetStartSamplePosition());
	EXPECT_EQ(400, transientPeakAndValley.GetStepSize());
	EXPECT_EQ(0, transientPeakAndValley.GetPeakPoint());
	EXPECT_EQ(0, transientPeakAndValley.GetValleyPoint());
	EXPECT_EQ(0, transientPeakAndValley.GetPlottedPoints().size());
}
*/
