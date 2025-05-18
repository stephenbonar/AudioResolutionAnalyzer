# CMakeLists.txt - CMakeLists for building the main program.
#
# Copyright (C) 2025 Stephen Bonar
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

# Define the directories that contain the header files the programs include.
set(INCLUDES 
    ${PROJECT_SOURCE_DIR}/libs/LibCppBinary/include
    ${PROJECT_SOURCE_DIR}/libs/LibCppCmdLine/include
    ${PROJECT_SOURCE_DIR}/libs/LibCppLogging/LibCppLogging
    ${PROJECT_SOURCE_DIR}/libs/flac/include/FLAC
    ${PROJECT_SOURCE_DIR}/libs/flac/include/FLAC++
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/src)

# Define the source files that make up the main program.
set(COMMON_SOURCES
    WaveFile.cpp
    FlacFile.cpp
    SampleDumper.cpp
    WaveFormat.cpp)

# Define the source files that make up the console program.
set(CONSOLE_SOURCES
    ConsoleMain.cpp 
    Program.cpp)

# Define the source files that make up the GUI program.
set(GUI_SOURCES
    Main.cpp
    AudioResolutionAnalyzer.cpp
    MainWindow.cpp
    AnalysisThread.cpp)

# Configure the program version info from the main cmake project into the
# Version.h header, which is build into the program binary. This is done so
# we centrally update the program name, version, and copyright from cmake.
configure_file(Version.h.in Version.h)

# Define the common libraries the the executables need to link with.
set(COMMON_LIBRARIES LibCppBinary LibCppCmdLine LibCppLogging FLAC++)

# Define the additional libraries the GUI needs to link with. 
set(GUI_LIBRARIES wx::net wx::core wx::base)

# Define the console executable target.
add_executable(analyzeaudio ${COMMON_SOURCES} ${CONSOLE_SOURCES})

# Define the Windows GUI executable target.
if(WIN32)
    add_executable(AudioResolutionAnalyzer WIN32 ${COMMON_SOURCES} ${GUI_SOURCES})
else()
    add_executable(AudioResolutionAnalyzer ${COMMON_SOURCES} ${GUI_SOURCES})
endif(WIN32)


# Include all the directories that contain headers that we need that are not
# in the current directory, otherwise the compiler won't find them
target_include_directories(analyzeaudio PUBLIC ${INCLUDES})

# Include all the directories that contain headers that we need that are not
# in the current directory, otherwise the compiler won't find them
target_include_directories(AudioResolutionAnalyzer PUBLIC ${INCLUDES})

# Configure the console target to link to the necessary libraries.
target_link_libraries(analyzeaudio ${COMMON_LIBRARIES})

# Configure the GUI library to link the necessary libraries.
target_link_libraries(
    AudioResolutionAnalyzer 
    ${COMMON_LIBRARIES} 
    ${GUI_LIBRARIES})