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

#include <gtest/gtest.h>
#include <Utilities/Timer.h>
#include <chrono>
#include <thread>

TEST(UtilitiesTimer, TestDoubleStart1)
{
	Utilities::Timer timer(Utilities::Timer::Action::START_NOW);
	ASSERT_THROW(timer.Start(), std::exception);
	try
	{
		timer.Start();	
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Attempting to start timer when already started", theException.what());
	}
}

TEST(UtilitiesTimer, TestDoubleStart2)
{
	Utilities::Timer timer;
	timer.Start();
	ASSERT_THROW(timer.Start(), std::exception);
	try
	{
		timer.Start();	
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Attempting to start timer when already started", theException.what());
	}
}

TEST(UtilitiesTimer, TestStopBeforeStart)
{
	Utilities::Timer timer;
	ASSERT_THROW(timer.Stop(), std::exception);
	try
	{
		timer.Stop();	
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Attempting to stop timer when not started", theException.what());
	}
}

TEST(UtilitiesTimer, TestDoubleStop)
{
	Utilities::Timer timer;
	timer.Start();
	timer.Stop();
	ASSERT_THROW(timer.Stop(), std::exception);
	try
	{
		timer.Stop();	
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Attempting to stop timer when not started", theException.what());
	}
}

TEST(UtilitiesTimer, TestTimer)
{
	Utilities::Timer timer(Utilities::Timer::Action::START_NOW);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	auto time = timer.Stop();
	std::cout << "UtilitiesTimer::TestTimer unit test time value: " << time << std::endl;
	EXPECT_TRUE(time > 0.8 && time < 1.20);  // Give us some room for overhead
}
