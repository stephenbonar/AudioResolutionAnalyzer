// SampleDumper.h - Declares the SampleDumper class.
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

#ifndef SAMPLE_DUMPER_H
#define SAMPLE_DUMPER_H

#include <string>
#include <sstream>
#include <memory>
#include "LibCppLogging.h"
#include "LibCppBinary.h"

class SampleDumper
{
public:
    SampleDumper(std::string fileName);

    void Dump(Binary::DataField* sample)
    {
        dumpLogger->Write(sample->ToString(Binary::StringFormat::Bin));
    }
private:
    std::unique_ptr<Logging::LogFile> dumpFile;
    std::unique_ptr<Logging::Logger> dumpLogger;
};

#endif