// Program.cpp - Defines the Program class methods and functions.
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

#include "Program.h"
#include "Version.h"

Program::Program(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
        arguments.push_back(std::string(argv[i]));

    DefineParams();
}

int Program::Run()
{
    PrintProgramInfo();

    if (!ParseArguments())
        return 1;

    if (inputFileParam->Value() == "debug")
    {
        Debug();
        return 0;
    }
    
    WaveFile inputFile{ inputFileParam->Value() };
    if (!inputFile.Exists())
    {
        std::cerr << "ERROR: " << inputFileParam->Value() << " does not exist!" << std::endl;
        return 1;
    }
    inputFile.Open();

    ConversionMethod method = ConversionMethod::LinearScaling;
    if (directCopyParam->IsSpecified())
        method = ConversionMethod::DirectCopy;

    if (to8BitOption->IsSpecified())
    {
        inputFile.Convert(outputFileParam->Value(), BitDepth::UInt8, method);
        return 0;
    }
    else if (to16BitOption->IsSpecified())
    {
        inputFile.Convert(outputFileParam->Value(), BitDepth::Int16, method);
        return 0;
    }
    else if (to24BitOption->IsSpecified())
    {
        inputFile.Convert(outputFileParam->Value(), BitDepth::Int24, method);
        return 0;
    }
    else if (to32BitOption->IsSpecified())
    {
        inputFile.Convert(outputFileParam->Value(), BitDepth::Int32, method);
        return 0;
    }
    else if (analyzeOption->IsSpecified())
    {
        inputFile.Analyze();
        PrintWaveInfo(inputFile);
        PrintAnalysisResults(inputFile);
        return 0;
    }
    else
    {
        return PrintWaveInfo(inputFile);
    }
}

void Program::DefineParams()
{
    CmdLine::ProgParam::Definition progDef;
    progDef.name = "analyzeaudio";
    progDef.description = "converts WAV files to different bit depths";
    progParam = std::make_shared<CmdLine::ProgParam>(progDef);

    CmdLine::PosParam::Definition inputFileDef;
    inputFileDef.name = "input-file";
    inputFileDef.description = "The file to use as input for the conversion";
    inputFileDef.isMandatory = true;
    inputFileParam = std::make_shared<CmdLine::PosParam>(inputFileDef);

    CmdLine::PosParam::Definition outputFileDef;
    outputFileDef.name = "output-file";
    outputFileDef.description = "The file to write the converted data to";
    outputFileDef.isMandatory = false;
    outputFileParam = std::make_shared<CmdLine::PosParam>(outputFileDef);

    CmdLine::Option::Definition analyzeDef;
    analyzeDef.shortName = 'a';
    analyzeDef.longName = "analyze";
    analyzeDef.description = "determines if the specified file was upscaled";
    analyzeOption = std::make_shared<CmdLine::Option>(analyzeDef);

    CmdLine::OptionParam::Definition directCopyDef;
    directCopyDef.name = "directcopy";
    directCopyDef.description = "uses direct value copy for conversion";
    directCopyDef.isMandatory = false;
    directCopyParam = std::make_shared<CmdLine::OptionParam>(directCopyDef);

    CmdLine::OptionParam::Definition linearScalingDef;
    linearScalingDef.name = "linearscale";
    linearScalingDef.description = "uses linear scaling for conversion";
    linearScalingDef.isMandatory = false;
    linearScalingParam = std::make_shared<CmdLine::OptionParam>(
        linearScalingDef);

    CmdLine::ValueOption::Definition methodDef;
    methodDef.shortName = 'm';
    methodDef.longName = "method";
    methodDef.description = "specifies the conversion method to use";
    methodOption = std::make_shared<CmdLine::ValueOption>(methodDef);
    methodOption->Add(directCopyParam.get());
    methodOption->Add(linearScalingParam.get());

    CmdLine::Option::Definition to8BitDef;
    to8BitDef.shortName = 'e';
    to8BitDef.longName = "8";
    to8BitDef.description = "converts the file to 8-bit audio";
    to8BitOption = std::make_shared<CmdLine::Option>(to8BitDef);

    CmdLine::Option::Definition to16BitDef;
    to16BitDef.shortName = 's';
    to16BitDef.longName = "16";
    to16BitDef.description = "converts the file to 16-bit audio";
    to16BitOption = std::make_shared<CmdLine::Option>(to16BitDef);

    CmdLine::Option::Definition to24BitDef;
    to24BitDef.shortName = 't';
    to24BitDef.longName = "24";
    to24BitDef.description = "converts the file to 24-bit audio";
    to24BitOption = std::make_shared<CmdLine::Option>(to24BitDef);

    CmdLine::Option::Definition to32BitDef;
    to32BitDef.shortName = 'r';
    to32BitDef.longName = "32";
    to32BitDef.description = "converts the file to 32-bit audio";
    to32BitOption = std::make_shared<CmdLine::Option>(to32BitDef);
}

bool Program::ParseArguments()
{
    CmdLine::Parser parser{ progParam.get(), arguments };
    parser.Add(inputFileParam.get());
    parser.Add(outputFileParam.get());
    parser.Add(analyzeOption.get());
    parser.Add(methodOption.get());
    parser.Add(to8BitOption.get());
    parser.Add(to16BitOption.get());
    parser.Add(to24BitOption.get());
    parser.Add(to32BitOption.get());
    CmdLine::Parser::Status status = parser.Parse();

    if (status == CmdLine::Parser::Status::Failure)
    {
        std::cerr << parser.GenerateUsage() << std::endl;
        std::cerr << "Invalid command line arguments specified!" << std::endl;
        return false;
    }  
    else if (parser.BuiltInHelpOptionIsSpecified())
    {
        std::cout << parser.GenerateHelp() << std::endl;
        return false;
    }
    else if (!parser.AllMandatoryParamsSpecified())
    {
        std::cout << parser.GenerateUsage() << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

void Program::PrintProgramInfo()
{
    std::cout << PROGRAM_NAME << " v" << VERSION_MAJOR << "." << VERSION_MINOR;
    std::cout << " " << PROGRAM_RELEASE << std::endl;
    std::cout << PROGRAM_COPYRIGHT << std::endl;
    std::cout << std::endl;
}

int Program::PrintWaveInfo(WaveFile& file)
{
    RiffChunkHeader header = file.GetChunkHeader();
    std::cout << "RIFF Chunk Header" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Chunk ID     : " << header.id << std::endl;
    std::cout << "Chunk Size   : " << header.size << std::endl;
    std::cout << "File Type    : " << header.type << std::endl;
    std::cout << std::endl;

    WaveFormat format = file.GetFormat();
    std::cout << "Format Info" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Audio Format : " << format.audioFormat << std::endl;
    std::cout << "Channels     : " << format.channels << std::endl;
    std::cout << "Sample Rate  : " << format.sampleRate << std::endl;
    std::cout << "Byte Rate    : " << format.byteRate << std::endl;
    std::cout << "Block Align  : " << format.blockAlign << std::endl;
    std::cout << "Bits / Sample: " << format.bitsPerSample << std::endl;
    std::cout << std::endl;

    return 0;
}

void Program::PrintAnalysisResults(WaveFile& file)
{
    std::cout << "Analysis Results" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    if (file.IsUpscaleConversion())
    {
        std::cout << "File appears to be an upscale conversion" << std::endl;
    }
    else
    {
        std::cout << "File appears to be a natural bit-depth" << std::endl;
    }
}

void Program::Debug()
{
    std::cout << "DEBUG" << std::endl;
    std::cout << "-----" << std::endl;

    int positive = 127;
    BinData::Int16Field positiveField{ positive };
    BinData::Int16Field shiftedPositiveField{ 0 };
    shiftedPositiveField.SetValue(positive << 8);
    std::cout << "127       : ";
    std::cout << positiveField.ToString(BinData::Format::Bin) << std::endl;
    std::cout << "127 << 8  : ";
    std::cout << shiftedPositiveField.ToString(BinData::Format::Bin);
    std::cout << std::endl << std::endl;;

    if ((positiveField.Value() ^ 0x00FF) == 0)
        std::cout << "Empty least significant byte on positive" << std::endl;
    else
        std::cout << (positiveField.Value() ^ 0x00FF) << std::endl;

    if ((shiftedPositiveField.Value() ^ 0xFF00) == 0)
        std::cout << "Empty least significant byte on shifted positive" << std::endl;

    int negative = -128;
    BinData::Int16Field negativeField{ negative };
    BinData::Int16Field shiftedNegativeField{ 0 };
    BinData::Int16Field maskedValueField{ 0 };
    shiftedNegativeField.SetValue(negative << 8);
    maskedValueField.SetValue(shiftedNegativeField.Value() & 0x00FF);
    std::cout << "-128      : " << negativeField.ToString(BinData::Format::Bin) << std::endl;
    std::cout << "-128 << 8 : ";
    std::cout << shiftedNegativeField.ToString(BinData::Format::Bin);
    std::cout << std::endl << std::endl;

    std::cout << "Masked    : ";
    std::cout << maskedValueField.ToString(BinData::Format::Bin) << std::endl;
}