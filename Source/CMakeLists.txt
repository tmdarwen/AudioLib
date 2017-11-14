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

# "Externals" consists of GoogleTest
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeSupport/CMakeLists.Externals.txt)
include_externals()

add_subdirectory(AudioData)
add_subdirectory(Signal)
add_subdirectory(ThreadSafeAudioFile)
add_subdirectory(Utilities)
add_subdirectory(WaveFile)
