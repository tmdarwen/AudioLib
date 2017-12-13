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

#pragma once

#include <exception>
#include <string>
#include <Utilities/Stringify.h>

//! @file Exception.h
//! @brief Simple wrapper for std::exception.

namespace Utilities {

//! Simple wrapper class around std::exception.

class Exception : public std::exception
{
	public:

		//! Instantiate an exception without any information.
		Exception();

		//! @brief Instantiate an exception with specific information.
		//! @param what String information concerning this exception.
		//! @param file The source code file the exception originated in.
		//! @param lineNumber The line in the source code file the exception originated in.
		Exception(const std::string& what, const std::string& file="", std::size_t lineNumber=0);

		//! Get any string information describing this exception.
		const char* what() const noexcept override;
	
	private:
	    std::string what_;
	    std::string file_;
	    std::size_t lineNumber_;
		mutable std::string extendedWhat_;
};

//! @brief Throw an exception without having to construct the exception object yourself.
//! @param params A variable list of string information to describe the exception.
template<typename... Params>
void ThrowException(Params... params)
{
	std::string message = Utilities::CreateString("|", params...);
	throw Utilities::Exception(message);
}

} // End of namespace
