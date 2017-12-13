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

#include <Utilities/Exception.h>
#include <Utilities/Stringify.h>

Utilities::Exception::Exception()
{

}

Utilities::Exception::Exception(const std::string& what, const std::string& file, std::size_t lineNumber) :
	what_(what),
	file_(file),
	lineNumber_(lineNumber)
{

}

const char* Utilities::Exception::what() const noexcept
{
	if(file_.size())
	{
		// The extendedWhat_ member is necessary so that when we return the char* the memory exists
		// outside the scope of this if and function as opposed to just calling a c_str() on a
		// Utilities::Stringify object that will be destroyed once we leave the if.
		extendedWhat_ = Utilities::Stringify(what_.c_str()) +
								  Utilities::Stringify(" ") +
								  file_ + Utilities::Stringify(":") +
								  Utilities::Stringify(lineNumber_);
		return extendedWhat_.c_str();
	}

	return what_.c_str();
}
