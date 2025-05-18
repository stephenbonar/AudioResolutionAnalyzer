# CMakeLists.txt - CMakeLists for building the project.
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

cmake_minimum_required(VERSION 3.12)
project(AudioResolutionAnalyzer VERSION 0.75.0 LANGUAGES CXX)

# Although the project command can natively pass the name and version into
# the program through the configure_file command, there is no native RELEASE
# or COPYRIGHT info, so we define that here to pass to configure_file.
set(PROJECT_RELEASE "Alpha")
set(PROJECT_COPYRIGHT "Copyright (C) 2025 Stephen Bonar")

# Specify the C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tell the FLAC library not to build with support for unnecessary dependencies,
# such as OGG support and man pages.
set(WITH_OGG OFF)
set(INSTALL_MANPAGES OFF)
set(BUILD_PROGRAMS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)

# By default, GoogleTest expects to statically link to the C++ runtime on the
# MSVC compiler. So if we run on Windows, we need to set our compiler flags
# to statically link the rest of the project to the runtime to match.
if (WIN32)
    # We need the C compiler flags set to static even if we use the C++
    # FLAC library because the C++ library is just a wrapper for the C library.
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")

    # Also set the C++ flags for static linking with the runtime.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# Add the LibCppBinData library to the project.
add_subdirectory(libs/LibCppBinary)

# Add the LibCppCmdLine library to the project.
add_subdirectory(libs/LibCppCmdLine)

# Add the LibCppLogging library to the project.
add_subdirectory(libs/LibCppLogging)

# Add the FLAC library to the project.
add_subdirectory(libs/flac)

# Add wxWidgets to the project.
add_subdirectory(libs/wxWidgets)

# Add the AudioResolutionAnalyzer program to the project.
add_subdirectory(src)

# Add the prototype version of AudioResolutionAnalyzer to the project.
add_subdirectory(src/prototype)
