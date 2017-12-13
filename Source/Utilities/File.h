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

#include <string>
#include <vector>
#include <fstream>

//! @file File.h
//! @brief Class and functions for allowing easy file reading access.

namespace Utilities { namespace File {

//! Returns the dirname of the given path and filename
std::string GetDirname(const std::string& filenameWithPath);

//! Returns the basename of the given path and filename
std::string GetBasename(const std::string& filenameWithPath);

//! Removes the extension from the given filename
void RemoveExtension(std::string& filename);

//! A fairly simple class to read data from a file.

class FileReader
{
	public:
		//! Construct the FileReader.
		FileReader(const std::string filename);
		virtual ~FileReader();

		//! Get the number of bytes contained in the file.
		std::size_t GetFileSize();

		//! @brief Read data from the file.
		//! @param position Byte position in the file to start the read.
		//! @param bytes Number of bytes to read.
		std::vector<char> ReadData(std::size_t position, std::size_t bytes);

	private:
		std::string filename_;
		std::ifstream fileStream_;
		std::size_t fileSize_;

		void InitializeFileReading();
};

//! Diffs two files returning true if they match and false otherwise.
bool CheckIfFilesMatch(const std::string& filenameA, const std::string& filenameB);

//! Diffs two files.  The diffInfo will contain information about the differences if the files do not match.
bool CheckIfFilesMatch(const std::string& filenameA, const std::string& filenameB, std::string& diffInfo);

}} // End of namespace
