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

#include <Utilities/File.h>
#include <Utilities/Exception.h>

std::size_t GetFilenameStartPosition(const std::string& filenameWithPath)
{
	auto slashPosition = filenameWithPath.find_last_of("/");
	auto backslashPosition = filenameWithPath.find_last_of("\\");
	if(slashPosition == std::string::npos && backslashPosition == std::string::npos)
	{
		return 0;
	}
	else if(slashPosition == std::string::npos)
	{
		return backslashPosition;
	}
	else if(backslashPosition == std::string::npos)
	{
		return slashPosition;
	}
	else if(slashPosition > backslashPosition)
	{
		return slashPosition;
	}
	else
	{
		return backslashPosition;
	}
}

std::string Utilities::File::GetDirname(const std::string& filenameWithPath)
{
	return filenameWithPath.substr(0, GetFilenameStartPosition(filenameWithPath));
}

std::string Utilities::File::GetBasename(const std::string& filenameWithPath)
{
	auto finalSlashStartPosition = GetFilenameStartPosition(filenameWithPath);
	if(finalSlashStartPosition == 0)
	{
		// If we're here, there was no path, just a filename
		return filenameWithPath;
	}

	return filenameWithPath.substr(GetFilenameStartPosition(filenameWithPath) + 1);
}

void Utilities::File::RemoveExtension(std::string& filename)
{
	auto position = filename.find_last_of(".");
	if(position != std::string::npos)
	{
		filename = filename.substr(0, position);
	}
}

Utilities::File::FileReader::FileReader(const std::string filename) : 
	filename_{filename}, 
	fileStream_(filename_, std::ios::in | std::ios::binary)
{
	InitializeFileReading();
}

Utilities::File::FileReader::~FileReader()
{
	fileStream_.close();
}

std::size_t Utilities::File::FileReader::GetFileSize()
{
	return fileSize_;
}

std::vector<char> Utilities::File::FileReader::ReadData(std::size_t position, std::size_t bytes)
{
	std::vector<char> data(bytes);

	fileStream_.seekg(position, std::ios_base::beg);

	fileStream_.read(data.data(), bytes);
	if(!fileStream_.good())
	{
		Utilities::Exception(Utilities::Stringify("Failed to read audio data from file " + filename_));
	}

	return data;
 }

void Utilities::File::FileReader::InitializeFileReading()
{
	if(!fileStream_.is_open())
	{
		Utilities::ThrowException("Failed to open file", filename_, __FILE__, __LINE__);
	}

	fileStream_.seekg(0, std::ios_base::end);
	fileSize_ = static_cast<std::size_t>(fileStream_.tellg());
	fileStream_.seekg(0, std::ios_base::beg);
}

bool Utilities::File::CheckIfFilesMatch(const std::string& fileA, const std::string& fileB)
{
	std::string diffInfo;
	return CheckIfFilesMatch(fileA, fileB, diffInfo);
}

bool Utilities::File::CheckIfFilesMatch(const std::string& fileA, const std::string& fileB, std::string& diffInfo)
{
	diffInfo.clear();

	Utilities::File::FileReader fileReaderA{fileA};
	Utilities::File::FileReader fileReaderB{fileB};

	if(fileReaderA.GetFileSize() != fileReaderB.GetFileSize())
	{
		diffInfo  = Utilities::CreateString("|", "File sizes differ", fileA, fileB, fileReaderA.GetFileSize(), fileReaderB.GetFileSize());
		return false;
	}

	std::size_t currentPosition{0};
	const std::size_t readSize{1024};
	while(currentPosition < fileReaderA.GetFileSize())
	{
		std::size_t readAmount{readSize};
		if((currentPosition + readAmount) > fileReaderA.GetFileSize())
		{
			readAmount = fileReaderA.GetFileSize() - currentPosition;	
		}

		auto bufferA{fileReaderA.ReadData(currentPosition, readAmount)};
		auto bufferB{fileReaderB.ReadData(currentPosition, readAmount)};

		for(std::size_t i{0}; i < readAmount; ++i)
		{
			if(bufferA[i] != bufferB[i])
			{
				diffInfo  = Utilities::CreateString(" ", "Byte values differ", fileA, fileB, "byte number", currentPosition + i);
				return false;
			}
		}

		currentPosition += readAmount;
	}

	return true;
}

