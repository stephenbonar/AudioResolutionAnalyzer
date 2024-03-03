// RiffSubChunkHeader.h - Declares the RiffSubChunkHeader struct.
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

#ifndef RIFF_SUB_CHUNK_HEADER_H
#define RIFF_SUB_CHUNK_HEADER_H

#include "BinData.h"

struct RiffSubChunkHeader
{
    // Common values include 'fmt ', 'info', and 'data'.
    BinData::StringField id{ 4 };

    // Indicates the size of the rest of the subchunk after this header.
    BinData::UInt32Field size{ 0 };
};

#endif