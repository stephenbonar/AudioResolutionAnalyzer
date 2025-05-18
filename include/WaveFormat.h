// WaveFormat.h - Declares the WaveFormat struct.
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

#ifndef WAVE_FORMAT_H
#define WAVE_FORMAT_H

#include <vector>
#include "LibCppBinary.h"

struct WaveFormat : public Binary::DataStructure
{
    Binary::UInt16Field audioFormat{ 0 };
    Binary::UInt16Field channels{ 0 };
    Binary::UInt32Field sampleRate{ 0 };
    Binary::UInt32Field byteRate{ 0 };
    Binary::UInt16Field blockAlign{ 0 };
    Binary::UInt16Field bitsPerSample{ 0 };

    WaveFormat();

    std::vector<Binary::DataField*> Fields() override { return fields; }

    size_t Size() const override;
private:
    std::vector<Binary::DataField*> fields;
};

#endif