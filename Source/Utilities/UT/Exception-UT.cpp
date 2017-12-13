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
#include <Utilities/Exception.h>

void TestThrow()
{
	Utilities::Exception theException("Throwing");
	throw theException;
}

void TestThrowWithFilename()
{
	Utilities::Exception theException("Throwing", "SomeFilename");
	throw theException;
}

void TestThrowWithFilenameAndLineNumber()
{
	Utilities::Exception theException("Throwing", "SomeFilename", 99);
	throw theException;
}

TEST(UtilitiesException, TestThrow)
{
	// Since Utilities::Exception inherits from std::exception, it's considered both types
	ASSERT_THROW(TestThrow(), std::exception);
	ASSERT_THROW(TestThrow(), Utilities::Exception);
}

TEST(UtilitiesException, CatchWhat)
{
	try
	{
		TestThrow();
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Throwing", theException.what());
	}

	try
	{
		TestThrow();
	}
	catch(Utilities::Exception& theException)
	{
		EXPECT_STREQ("Throwing", theException.what());
	}
}

TEST(UtilitiesException, TestThrowWithFilename)
{
	try
	{
		TestThrowWithFilename();
	}
		catch(std::exception& theException)
	{
		EXPECT_STREQ("Throwing SomeFilename:0", theException.what());
	}

	try
	{
		TestThrowWithFilename();
	}
		catch(Utilities::Exception& theException)
	{
		EXPECT_STREQ("Throwing SomeFilename:0", theException.what());
	}
}

TEST(UtilitiesException, TestThrowWithFilenameAndLineNumber)
{
	try
	{
		TestThrowWithFilenameAndLineNumber();
	}
	catch(std::exception& theException)
	{
		EXPECT_STREQ("Throwing SomeFilename:99", theException.what());
	}

	try
	{
		TestThrowWithFilenameAndLineNumber();
	}
	catch(Utilities::Exception& theException)
	{
		EXPECT_STREQ("Throwing SomeFilename:99", theException.what());
	}
}
