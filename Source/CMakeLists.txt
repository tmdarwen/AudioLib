cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(AudioLib)

if(DEBUG_BUILD)
	add_definitions(-DDEBUG_BUILD)
endif(DEBUG_BUILD)

if(DEBUG_TO_LOG_FILE)
	add_definitions(-DDEBUG_BUILD)
	add_definitions(-DDEBUG_TO_LOG_FILE)
endif(DEBUG_TO_LOG_FILE)

option(INCLUDE_GOOGLE_TEST "Pulldown GoogleTest and include it in the project" ON)

option(MSVC_MULTI_THREADED_DLL_RUNTIME_LIB "Allows for setting the runtime lib for MSVC compiler to multi-threaded DLL" OFF)

# "Externals" consists of GoogleTest
if(INCLUDE_GOOGLE_TEST)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeSupport/CMakeLists.Externals.txt)
	include_externals()
endif()

add_subdirectory(AudioData)
add_subdirectory(Signal)
add_subdirectory(ThreadSafeAudioFile)
add_subdirectory(Utilities)
add_subdirectory(WaveFile)
