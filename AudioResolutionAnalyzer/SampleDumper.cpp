// SampleDumper.cpp - Defines the SampleDumper class methods.
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
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SampleDumper.h"

SampleDumper::SampleDumper(std::string fileName)
{
    std::stringstream pathStream;
    std::filesystem::path path = std::filesystem::path{ fileName };
    pathStream << path.filename().string() << ".samples.txt";
    dumpLogger = std::make_unique<Logging::Logger>();
    dumpFile = std::make_unique<Logging::LogFile>(pathStream.str());
    Logging::ChannelSettings dumpSettings = dumpFile->Settings();
    dumpSettings.includeTimestamp = false;
    dumpSettings.includeLogLevel = false;
    dumpFile->SetSettings(dumpSettings);
    dumpLogger->Add(dumpFile.get());
}