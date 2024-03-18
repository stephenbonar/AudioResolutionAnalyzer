// Program.h - Declares the Program class.
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

#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <iostream>
#include <memory>
#include "CmdLine.h"
#include "WaveFile.h"
#include "BitDepth.h"
#include "ConversionMethod.h"

class Program
{
public:
    Program(int argc, char** argv);

    int Run();
private:
    std::vector<std::string> arguments;
    std::shared_ptr<CmdLine::ProgParam> progParam;
    std::shared_ptr<CmdLine::PosParam> inputFileParam;
    std::shared_ptr<CmdLine::PosParam> outputFileParam;
    std::shared_ptr<CmdLine::Option> analyzeOption;
    std::shared_ptr<CmdLine::ValueOption> methodOption;
    std::shared_ptr<CmdLine::OptionParam> directCopyParam;
    std::shared_ptr<CmdLine::OptionParam> linearScalingParam;
    std::shared_ptr<CmdLine::ValueOption> convertOption;
    std::shared_ptr<CmdLine::OptionParam> to8BitParam;
    std::shared_ptr<CmdLine::OptionParam> to16BitParam;
    std::shared_ptr<CmdLine::OptionParam> to24BitParam;
    std::shared_ptr<CmdLine::OptionParam> to32BitParam;

    void DefineParams();

    bool ParseArguments();

    void PrintProgramInfo();

    int PrintWaveInfo(WaveFile& file);

    void PrintAnalysisResults(WaveFile& file);

    void Debug();
};

#endif