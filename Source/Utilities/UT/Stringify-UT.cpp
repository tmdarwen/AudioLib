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
#include <Utilities/Stringify.h>

TEST(Stringify, TestStringify)
{
	int intValue(3);
	std::string theString("The magic number is " + Utilities::Stringify(intValue));
	EXPECT_STREQ("The magic number is 3", theString.c_str());
}

TEST(Stringify, StringifyCharacters)
{
	// Unfortunately, I have not yet figured out how to Stringify characters.  The following line does not work:
	//std::string theString(Utilities::Stringify('H') + Utilities::Stringify('e'));
	std::string theString(Utilities::Stringify("H") + Utilities::Stringify("e"));
	EXPECT_STREQ("He", theString.c_str());
}

TEST(Stringify, TestCreateStringWithSpaceDelimiter)
{
	std::string theString = Utilities::CreateString(" ", "Hello", "World", 1, 2, "Three");
	EXPECT_STREQ("Hello World 1 2 Three", theString.c_str());
}

TEST(Stringify, TestCreateStringWithPipeDelimiter)
{
	std::string theString = Utilities::CreateString("|", 1, "Two", 3, "Four", 5);
	EXPECT_STREQ("1|Two|3|Four|5", theString.c_str());
}

TEST(Stringify, TestDelimitedStringToVectorOfStrings)
{
	std::string theString("Hello-World-One-Two");
	auto parsedResult = Utilities::DelimitedStringToVectorOfStrings(theString, '-');

	EXPECT_EQ(4, parsedResult.size());
	EXPECT_STREQ("Hello", parsedResult[0].c_str());
	EXPECT_STREQ("World", parsedResult[1].c_str());
	EXPECT_STREQ("One", parsedResult[2].c_str());
	EXPECT_STREQ("Two", parsedResult[3].c_str());
}

TEST(Stringify, TestToUpper)
{
	std::string theString("Hello World");
	auto uc = Utilities::ConvertStringToUppercase(theString);

	EXPECT_STREQ("HELLO WORLD", uc.c_str());
}
