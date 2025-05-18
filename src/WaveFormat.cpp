// WaveFormat.cpp - Defines the WaveFormat struct.
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

#include "WaveFormat.h"

WaveFormat::WaveFormat()
{
    fields.push_back(&audioFormat);
    fields.push_back(&channels);
    fields.push_back(&sampleRate);
    fields.push_back(&byteRate);
    fields.push_back(&blockAlign);
    fields.push_back(&bitsPerSample);
}

size_t WaveFormat::Size() const
{
    size_t size;

    for (size_t i = 0; i < fields.size(); i++)
        size += fields[i]->Size();

    return size;
}