#include <gtest/gtest.h>
#include <Utilities/File.h>

//////////////////////////////////////////////////////////////////////////////////
// GetDirname Tests

TEST(FileUtilities, GetDirnameTestNoPath)
{
	EXPECT_EQ("", Utilities::File::GetDirname("nada"));
}

TEST(FileUtilities, GetDirnameTestTypicalNixPath)
{
	EXPECT_EQ("/usr/share/applications", Utilities::File::GetDirname("/usr/share/applications/nada"));
}

TEST(FileUtilities, GetDirnameTestTypicalWindowsPaths)
{
	EXPECT_EQ("C:\\This\\Is\\A\\Path", Utilities::File::GetDirname("C:\\This\\Is\\A\\Path\\nada.exe"));
}

TEST(FileUtilities, GetDirnameTestPathMixture)
{
	EXPECT_EQ("C:\\This\\Is/A/Path", Utilities::File::GetDirname("C:\\This\\Is/A/Path/nada.exe"));
}

//////////////////////////////////////////////////////////////////////////////////
// GetBasename Tests

TEST(FileUtilities, GetBasenameTestNoPath)
{
	EXPECT_EQ("nada", Utilities::File::GetBasename("nada"));
}

TEST(FileUtilities, GetBasenameTestTypicalNixPath)
{
	auto x = Utilities::File::GetBasename("/usr/share/applications/nada");
	EXPECT_EQ("nada", x);
	//EXPECT_EQ("nada", Utilities::File::GetBasename("/usr/share/applications/nada"));
}

TEST(FileUtilities, GetBasenameTestTypicalWindowsPaths)
{
	EXPECT_EQ("nada.exe", Utilities::File::GetBasename("C:\\This\\Is\\A\\Path\\nada.exe"));
}

TEST(FileUtilities, GetBasenameTestPathMixture)
{
	EXPECT_EQ("nada.exe", Utilities::File::GetBasename("C:\\This\\Is/A/Path/nada.exe"));
}

//////////////////////////////////////////////////////////////////////////////////
// RemoveExtension Tests

TEST(FileUtilities, TestTypicalRemoveExtension)
{
	std::string myFilename("MyFilename.txt");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("MyFilename", myFilename);
}

TEST(FileUtilities, TestTypicalRemoveExtensionWithWindowsPath)
{
	std::string myFilename("C:\\Some\\Path\\To\\The\\File\\MyFilename.txt");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("C:\\Some\\Path\\To\\The\\File\\MyFilename", myFilename);
}

TEST(FileUtilities, TestTypicalRemoveExtensionWithUnixPath)
{
	std::string myFilename("/usr/local/hello/world/MyFilename.txt");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("/usr/local/hello/world/MyFilename", myFilename);
}

TEST(FileUtilities, TestRemoveExtensionWithNoExtension)
{
	std::string myFilename("MyFilename");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("MyFilename", myFilename);
}

TEST(FileUtilities, TestRemoveExtensionWithNoExtensionAndWindowsPath)
{
	std::string myFilename("C:\\Some\\Path\\To\\The\\File\\MyFilename");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("C:\\Some\\Path\\To\\The\\File\\MyFilename", myFilename);
}

TEST(FileUtilities, TestRemoveExtensionWithNoExtensionAndUnixPath)
{
	std::string myFilename("/usr/local/hello/world/MyFilename");
	Utilities::File::RemoveExtension(myFilename);
	EXPECT_EQ("/usr/local/hello/world/MyFilename", myFilename);
}

//////////////////////////////////////////////////////////////////////////////////
// FileReader Tests

TEST(FileUtilities, TestNonExistantFile)
{
	EXPECT_THROW(Utilities::File::FileReader fileReader("NonExistantFile"), std::exception);	
}

TEST(FileUtilities, TestFileSize)
{
	Utilities::File::FileReader fileReaderA("TestFileA.txt");
	EXPECT_EQ(19, fileReaderA.GetFileSize());

	Utilities::File::FileReader fileReaderB("TestFileB.txt");
	EXPECT_EQ(60, fileReaderB.GetFileSize());
}

TEST(FileUtilities, TestFileReading)
{
	Utilities::File::FileReader fileReader("TestFileA.txt");

	auto dataRead{fileReader.ReadData(0, 6)};

	EXPECT_EQ(6, dataRead.size());
	EXPECT_EQ('T', dataRead[0]);
	EXPECT_EQ('h', dataRead[1]);
	EXPECT_EQ('i', dataRead[2]);
	EXPECT_EQ('s', dataRead[3]);
	EXPECT_EQ(' ', dataRead[4]);
	EXPECT_EQ('i', dataRead[5]);
}

TEST(FileUtilities, FileMatchingTest1)
{
	std::string diffInfo;
	EXPECT_FALSE(Utilities::File::CheckIfFilesMatch("TestFileA.txt", "TestFileB.txt", diffInfo));
	EXPECT_STREQ("File sizes differ|TestFileA.txt|TestFileB.txt|19|60", diffInfo.c_str());
}

TEST(FileUtilities, FileMatchingTest2)
{
	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("TestFileA.txt", "TestFileA.txt"));
	EXPECT_TRUE(Utilities::File::CheckIfFilesMatch("TestFileB.txt", "TestFileB.txt"));
}
