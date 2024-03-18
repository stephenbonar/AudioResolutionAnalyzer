# CMakeLists.txt - CMakeLists for building the main program.
#
# Copyright (C) 2024 Stephen Bonar
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http ://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissionsand
# limitations under the License.

# Define the directories that contain the header files the tests include.
set(INCLUDES 
    ${PROJECT_SOURCE_DIR}/LibCppBinData/LibCppBinData
    ${PROJECT_SOURCE_DIR}/LibCppCmdLine/LibCppCmdLine
    ${PROJECT_SOURCE_DIR}/flac/include/FLAC
    ${PROJECT_SOURCE_DIR}/flac/include/FLAC++
    ${PROJECT_BINARY_DIR}/AudioResolutionAnalyzer)

# Configure the program version info from the main cmake project into the
# Version.h header, which is build into the program binary. This is done so
# we centrally update the program name, version, and copyright from cmake.
configure_file(Version.h.in Version.h)

# Define the libraries the tests need to link against.
set(LIBRARIES LibCppBinData LibCppCmdLine FLAC++)

# Define the executable targets.
add_executable(analyzeaudio ConsoleMain.cpp Program.cpp WaveFile.cpp)

# Include all the directories that contain headers that we need that are not
# in the current directory, otherwise the compiler won't find them
target_include_directories(analyzeaudio PUBLIC ${INCLUDES})

# Configure the tunebeepertests target to link to the necessary libraries
target_link_libraries(analyzeaudio ${LIBRARIES})