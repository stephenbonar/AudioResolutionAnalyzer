// MediaFile.cpp - Declares the MediaFile base class.
//
// Copyright (C) 2024 Stephen Bonar
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
// See the License for the specific language governing permissionsand
// limitations under the License.

#ifndef MEDIA_FILE_H
#define MEDIA_FILE_H

#include <string>
#include "BitDepth.h"
#include "ConversionMethod.h"

class MediaFile
{
public:
    /// @brief Destructs a MediaFile.
    virtual ~MediaFile() = default;

    virtual void Open() = 0;

    virtual bool Exists() = 0;

    virtual void Analyze() = 0;

    virtual void Convert(
        std::string outputFileName, 
        BitDepth depth, 
        ConversionMethod method) = 0;

    virtual bool IsUpscaleConversion() = 0;
};

#endif