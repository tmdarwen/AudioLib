call "%VisualStudioPath%\vcvarsall.bat" amd64
cd %WORKSPACE%
mkdir AudioLibBuilt
cd AudioLibBuilt
cmake -G "%CMakeGeneratorString%" ../Source
devenv AudioLib.sln /Build Debug
devenv AudioLib.sln /Build Release
