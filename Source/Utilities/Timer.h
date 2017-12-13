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

#include <chrono>
#include <ctime>
#include <atomic>

//! @file Timer.h
//! @brief Class to allow for doing timing between lines of code.

namespace Utilities {

//! Class to allow for doing timing between lines of code.

class Timer {

	public:
		//! Action to pass to the consutrctor to start timing now immediately.
		enum class Action { START_NOW };

		//! Instantiate the Timer.
		Timer();

		//! Instantiate the Timer, starting the timer running at the time of instantiation.
		Timer(Action);

		//! Start the timer.
	    void Start();

		//! Stop the timer, returning the number of seconds accumulated.
	    double Stop();
	
	private:
	    std::chrono::time_point<std::chrono::system_clock> startTime_;
		std::atomic_bool started_{false};
};

} // End of namespace
