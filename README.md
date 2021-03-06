AudioLib
========

This is a library I've made to support some of my audio related projects.  It's currently used in my [PhaseVocoder](https://github.com/tmdarwen/PhaseVocoder) project and my [AudioAnalysisTool](https://github.com/tmdarwen/AudioAnalysisTool) project.  Feel free to use this library in your own projects.  It's licensed under the permissable [MIT license](https://en.wikipedia.org/wiki/MIT_License).

 

**Main Components**

-   AudioData - A class allowing for easy transportation and access of digital audio data. 

-   Signal - Various digital signal processing utilities, such as the [PhaseVocder](Documentation/HowThePhaseVocoderWorks.md) and [TransientDetector](Documentation/TransientDetection.md).

-   ThreadSafeAudioFile - Provides threadsafe reading and writing of wave audio files.

-   Utilities - Basic utilities supporting application development.

-   WaveFile - Simple classes for reading and writing wave audio files.

 

**Doxygen Documentation**

Doxygen documentation can easily be generated by simply running Doxygen using the config file located in the [Doxygen](Doxygen) directory.

 

**Unit Test Coverage**

Unit test coverage is extensive.  You'll notice every component within the source directory has a UT directory which contains unit tests.  These automatically build and run as part of the build process.

 

**Build Dependencies**

-   Building this project requires [CMake](https://cmake.org) version 3.0 or later.

-   Building this project requires a C++14 compliant compiler.  This project is routinely built in my continous integration system which uses MS Visual Studio 2017, GCC 5.3.1 and Apple LLVM version 7.3.0 (clang 703.0.31).

-   [GoogleTest](https://github.com/google/googletest) is currently the only external dependency.  You do *not* need to clone or install this manually. The GoogleTest GitHub repo will be cloned automatically when CMake runs.

 

**Steps for Building**

1.   Clone this repo.

2.   Create a new directory at the parallel level as the cloned repo.  This directory will hold the project files CMake creates.

3.   cd into this new directory.

4.   From the command line, run _cmake -G YourDesiredGeneratorType ../AudioLib/Source_

5.   Run make or open the project file in an IDE and build.

 


**Continuous Integration**

The [Automation/CI directory](/Automation/CI) contains scripts that can be used with [Jenkins](https://jenkins.io/) to setup continuous integration in Linux, MacOS and Windows.  For more information on how to setup Jenkins to use these scripts please see [this document](https://github.com/tmdarwen/PhaseVocoder/tree/master/Documentation/JenkinsSetup.md).

 


**Licensing**

The MIT License applies to this software and its supporting documentation:

*Copyright (c) 2017-2018 - Terence M. Darwen - tmdarwen.com*

*Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:*

*The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.*

*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*
