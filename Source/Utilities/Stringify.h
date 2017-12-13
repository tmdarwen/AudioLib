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

#include <string>
#include <vector>

//! @file Stringify.h
//! @brief Functions designed to make string concatenation and conversion easy.

namespace Utilities {

//! @brief Stringify a fundamental type object.
//! @param t The object to stringify.
template<class T>
typename std::enable_if<std::is_fundamental<T>::value, std::string>::type Stringify(const T& t)
{
	return std::to_string(t);
}

//! @brief Stringify a non-fundamental type object.
//! @param t The object to stringify.
template<class T>
typename std::enable_if<!std::is_fundamental<T>::value, std::string>::type Stringify(const T& t)
{
	return std::string(t);
}

//! @brief The terminating variadic case for CreateString.
//! @param delimiter Delimiter used when concatenating given strings.
//! @param param The data to concatenate.
template<typename T>
std::string CreateString(const std::string& delimiter, T param)
{
	return Utilities::Stringify(param);
}

//! @brief Allows for concatenating a number of items together into a delimted string.
//! @param delimiter Delimiter used when concatenating given strings.
//! @param head The first parameter to concatenate.
//! @param tail The variable remaining parameters to concatenate.
template<typename T, typename... Tail>
std::string CreateString(const std::string& delimiter, T head, Tail... tail)
{
	return Utilities::Stringify(head) + Utilities::Stringify(delimiter) + Utilities::CreateString(delimiter, tail...);	
}

//! @brief Convert a string of delimted values into separate values in a std::vector.
//! @param delimitedString The string to parse.
//! @param delimiter Delimiter identifier for parsing the string.
std::vector<std::string> DelimitedStringToVectorOfStrings(const std::string &delimitedString, char delimiter);

//! @brief Convert a string to all uppercase.
//! @param inputString The string to convert to uppercase.
std::string ConvertStringToUppercase(const std::string &inputString);

}  // End of namespace
