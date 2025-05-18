// MediaFile.h - Declares the MediaFile base class.
//
// Copyright (C) 2025 Stephen Bonar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MEDIA_FILE_H
#define MEDIA_FILE_H

#include <string>
#include <algorithm>
#include <stdexcept>
#include "BitDepth.h"
#include "ConversionMethod.h"
#include "MediaFileType.h"
#include "LibCppLogging.h"

class MediaFormatError : public std::runtime_error
{
public:
    MediaFormatError(const char* message) : std::runtime_error{ message } { }
};

class MediaFile
{
public:
    /// @brief Destructs a MediaFile.
    virtual ~MediaFile() = default;

    virtual std::string FileName() const = 0;

    virtual int BitsPerSample() const = 0;

    virtual long SampleRate() const = 0;

    virtual bool IsOpen() const = 0;

    virtual void Open() = 0;

    virtual bool Exists() const { return std::filesystem::exists(FileName()); }

    virtual void Analyze(bool dumpSamples) = 0;

    virtual void Convert(
        std::string outputFileName, 
        BitDepth depth, 
        ConversionMethod method) = 0;

    virtual bool IsUpscaled() const = 0;
};

#endif