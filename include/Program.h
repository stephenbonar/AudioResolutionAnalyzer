// Program.h - Declares the Program class.
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

#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include "LibCppCmdLine.h"
#include "WaveFile.h"
#include "BitDepth.h"
#include "ConversionMethod.h"
#include "LibCppLogging.h"
#include "MediaFile.h"
#include "FlacFile.h"

class Program
{
public:
    static constexpr int ExitStatusSuccess{ 0 };
    static constexpr int ExitStatusInvalidArgsError{ 1 };
    static constexpr int ExitStatusInputFileError{ 2 };
    static constexpr int ExitStatusUnsupportedFile{ 3 };
    static constexpr int ExitStatusNotImplemented{ 4 };

    Program(int argc, char** argv);

    int Run();
private:
    std::vector<std::string> arguments;
    std::shared_ptr<CmdLine::ProgParam> progParam;
    std::shared_ptr<CmdLine::PosParam> inputFileParam;
    std::shared_ptr<CmdLine::PosParam> outputFileParam;
    std::shared_ptr<CmdLine::Option> analyzeOption;
    std::shared_ptr<CmdLine::ValueOption> methodOption;
    std::shared_ptr<CmdLine::Option> debugOption;
    std::shared_ptr<CmdLine::OptionParam> directCopyParam;
    std::shared_ptr<CmdLine::OptionParam> linearScalingParam;
    std::shared_ptr<CmdLine::ValueOption> convertOption;
    std::shared_ptr<CmdLine::OptionParam> to8BitParam;
    std::shared_ptr<CmdLine::OptionParam> to16BitParam;
    std::shared_ptr<CmdLine::OptionParam> to24BitParam;
    std::shared_ptr<CmdLine::OptionParam> to32BitParam;
    std::shared_ptr<CmdLine::Option> logOption;
    std::shared_ptr<CmdLine::Option> dumpOption;
    std::shared_ptr<Logging::StandardOutput> standardOutput;
    std::shared_ptr<Logging::StandardError> standardError;
    std::shared_ptr<Logging::LogFile> logFile;
    std::shared_ptr<Logging::Logger> logger;

    void DefineParams();

    bool ParseArguments();

    void PrintProgramInfo();

    void PrintSectionHeader(std::string text);

    void PrintField(
        std::string fieldName, 
        std::string value, 
        Logging::LogLevel level = Logging::LogLevel::Info);

    int PrintMediaInfo(MediaFile* file);

    int PrintWaveInfo(WaveFile* file);

    int PrintFlacInfo(FlacFile* file);

    void PrintAnalysisResults(MediaFile* file);

    std::shared_ptr<MediaFile> OpenFile(std::string fileName);
};

MediaFileType GetType(std::string fileName);

#endif