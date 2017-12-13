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

#include <Utilities/Timer.h>
#include <Utilities/Exception.h>

Utilities::Timer::Timer() { }

Utilities::Timer::Timer(Action)
{
	started_ = true;
	startTime_ = std::chrono::system_clock::now();
}

void Utilities::Timer::Start()
{
	if(started_)
	{
		Utilities::ThrowException("Attempting to start timer when already started");	
	}
	startTime_ = std::chrono::system_clock::now();
	started_ = true;
}
	
double Utilities::Timer::Stop()
{
	if(!started_)
	{
		Utilities::ThrowException("Attempting to stop timer when not started");	
	}
	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - startTime_;	
	started_ = false;
	return elapsed.count();
}
