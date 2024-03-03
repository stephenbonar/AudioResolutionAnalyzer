// RiffChunkHeader.h - Declares the RiffChunkHeader struct.
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

#ifndef RIFF_CHUNK_HEADER_H
#define RIFF_CHUNK_HEADER_H

#include "BinData.h"

struct RiffChunkHeader
{
    // Should be set to the characters 'RIFF' in a valid wave file.
    BinData::StringField id{ 4 };

    // The size of the rest of the file - 8 bytes (the id and size).
    BinData::UInt32Field size{ 0 };

    // Should be set to the characters 'WAVE' in a valid wave file.
    BinData::StringField type{ 4 };
};

#endif