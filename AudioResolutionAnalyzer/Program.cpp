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

    standardOutput = std::make_shared<Logging::StandardOutput>();
    standardError = std::make_shared<Logging::StandardError>();
    logFile = std::make_shared<Logging::LogFile>();

    // Standard output and error are included in the logger by default, but
    // the log file will only be included later if the -l option is specified.
    logger = std::make_shared<Logging::Logger>();
    logger->Add(standardOutput.get());
    logger->Add(standardError.get());
}

int Program::Run()
{
    PrintProgramInfo();

    if (!ParseArguments())
        return ExitStatusInvalidArgsError;

    if (logOption->IsSpecified())
    {
        logger->Add(logFile.get());
    }

    if (debugOption->IsSpecified())
    {
        Logging::ChannelSettings outputSettings = standardOutput->Settings();
        outputSettings.includeDebug = true;
        standardOutput->SetSettings(outputSettings);
        logFile->SetMinLogLevel(Logging::LogLevel::Debug);    
    }
    
    std::shared_ptr<MediaFile> inputFile = OpenFile(inputFileParam->Value());
    if (inputFile == nullptr)
        return ExitStatusInputFileError;

    ConversionMethod method = ConversionMethod::LinearScaling;
    if (directCopyParam->IsSpecified())
        method = ConversionMethod::DirectCopy;

    if (to8BitParam->IsSpecified())
    {
        inputFile->Convert(outputFileParam->Value(), BitDepth::UInt8, method);
        return ExitStatusSuccess;
    }
    else if (to16BitParam->IsSpecified())
    {
        inputFile->Convert(outputFileParam->Value(), BitDepth::Int16, method);
        return ExitStatusSuccess;
    }
    else if (to24BitParam->IsSpecified())
    {
        inputFile->Convert(outputFileParam->Value(), BitDepth::Int24, method);
        return ExitStatusSuccess;
    }
    else if (to32BitParam->IsSpecified())
    {
        inputFile->Convert(outputFileParam->Value(), BitDepth::Int32, method);
        return ExitStatusSuccess;
    }
    else if (analyzeOption->IsSpecified())
    {
        logger->Write("Analyzing file, please wait...");
        if (dumpOption->IsSpecified())
        {
            logger->Write(
                "NOTE: --dump-samples specified, this may take a while.");
        }
        logger->Write("");

        inputFile->Analyze(dumpOption->IsSpecified());
        PrintMediaInfo(inputFile.get());
        PrintAnalysisResults(inputFile.get());
        return ExitStatusSuccess;
    }
    else
    {
        return PrintMediaInfo(inputFile.get());
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

    CmdLine::ValueOption::Definition debugDef;
    debugDef.shortName = 'd';
    debugDef.longName = "debug";
    debugDef.description = "includes debug info in the log and on the screen";
    debugOption = std::make_shared<CmdLine::Option>(debugDef);

    CmdLine::OptionParam::Definition to8BitDef;
    to8BitDef.name = "8-bit";
    to8BitDef.description = "converts the file to 8-bit audio";
    to8BitParam = std::make_shared<CmdLine::OptionParam>(to8BitDef);

    CmdLine::OptionParam::Definition to16BitDef;
    to16BitDef.name = "16-bit";
    to16BitDef.description = "converts the file to 16-bit audio";
    to16BitParam = std::make_shared<CmdLine::OptionParam>(to16BitDef);

    CmdLine::OptionParam::Definition to24BitDef;
    to24BitDef.name = "24-bit";
    to24BitDef.description = "converts the file to 24-bit audio";
    to24BitParam = std::make_shared<CmdLine::OptionParam>(to24BitDef);

    CmdLine::OptionParam::Definition to32BitDef;
    to32BitDef.name = "32-bit";
    to32BitDef.description = "converts the file to 32-bit audio";
    to32BitParam = std::make_shared<CmdLine::OptionParam>(to32BitDef);

    CmdLine::ValueOption::Definition convertDef;
    convertDef.shortName = 'c';
    convertDef.longName = "convert";
    convertDef.description = "converts the file to the specified resolution";
    convertOption = std::make_shared<CmdLine::ValueOption>(convertDef);
    convertOption->Add(to8BitParam.get());
    convertOption->Add(to16BitParam.get());
    convertOption->Add(to24BitParam.get());
    convertOption->Add(to32BitParam.get());

    CmdLine::Option::Definition logDef;
    logDef.shortName = 'l';
    logDef.longName = "log";
    logDef.description = "writes messages to a log file, Log.txt";
    logOption = std::make_shared<CmdLine::Option>(logDef);

    CmdLine::Option::Definition dumpDef;
    dumpDef.shortName = 's';
    dumpDef.longName = "dump-samples";
    dumpDef.description = "dumps samples to a text file. Use with -a.";
    dumpOption = std::make_shared<CmdLine::Option>(dumpDef);
}

bool Program::ParseArguments()
{
    CmdLine::Parser parser{ progParam.get(), arguments };
    parser.Add(inputFileParam.get());
    parser.Add(outputFileParam.get());
    parser.Add(analyzeOption.get());
    parser.Add(convertOption.get());
    parser.Add(methodOption.get());
    parser.Add(logOption.get());
    parser.Add(debugOption.get());
    parser.Add(dumpOption.get());
    CmdLine::Parser::Status status = parser.Parse();

    if (status == CmdLine::Parser::Status::Failure)
    {
        logger->Write(parser.GenerateUsage());
        logger->Write(
            "Invalid command line arguments specified!", 
            Logging::LogLevel::Error);
        return false;
    }  
    else if (parser.BuiltInHelpOptionIsSpecified())
    {
        logger->Write(parser.GenerateHelp());
        return false;
    }
    else if (!parser.AllMandatoryParamsSpecified())
    {
        logger->Write(parser.GenerateUsage());
        return false;
    }
    else
    {
        return true;
    }
}

void Program::PrintProgramInfo()
{
    std::stringstream version;
    version 
        << PROGRAM_NAME << " v" << VERSION_MAJOR << "." << VERSION_MINOR
        << " " << PROGRAM_RELEASE;

    logger->Write(version.str());
    logger->Write(PROGRAM_COPYRIGHT);
    logger->Write("");
}

void Program::PrintSectionHeader(std::string text)
{
    logger->Write(text);
    logger->Write("----------------------------------------");
}

void Program::PrintField(
    std::string fieldName, 
    std::string value, 
    Logging::LogLevel level)
{
    std::stringstream field;
    field << std::setw(20) << std::left << fieldName << ": " << value;
    logger->Write(field.str(), level);
}

int Program::PrintMediaInfo(MediaFile* file)
{
    MediaFileType type = GetType(file->FileName());
    switch (type)
    {
        case MediaFileType::Wave:
        {
            WaveFile* waveFile = dynamic_cast<WaveFile*>(file);
            return PrintWaveInfo(waveFile);
        }
        case MediaFileType::Flac:
        {
            FlacFile* flacFile = dynamic_cast<FlacFile*>(file);
            return PrintFlacInfo(flacFile);
        }
        case MediaFileType::Unsupported:
        {
            return ExitStatusUnsupportedFile;
        }
    }
}

int Program::PrintWaveInfo(WaveFile* file)
{
    RiffChunkHeader header = file->ChunkHeader();
    PrintSectionHeader("RIFF Chunk Header");
    PrintField("Chunk ID", header.id.ToString());
    PrintField("Chunk Size", header.size.ToString());
    PrintField("File Type", header.type.ToString());
    logger->Write("");

    WaveFormat format = file->Format();
    PrintSectionHeader("Format Info");
    PrintField("Audio Format", format.audioFormat.ToString());
    PrintField("Channels", format.channels.ToString());
    PrintField("Sample Rate", format.sampleRate.ToString());
    PrintField("Byte Rate", format.byteRate.ToString());
    PrintField("Block Align", format.blockAlign.ToString());
    PrintField("Bits / Sample", format.bitsPerSample.ToString());
    logger->Write("");

    return ExitStatusSuccess;
}

int Program::PrintFlacInfo(FlacFile* file)
{
    FlacFormat format = file->Format();
    PrintSectionHeader("Format Info");
    PrintField("Channels", std::to_string(format.channels));
    PrintField("Sample Rate", std::to_string(format.sampleRate));
    PrintField("Block Size", std::to_string(format.blockSize));
    PrintField("Bits / Sample", std::to_string(format.bitsPerSample));
    PrintField("Total Samples", std::to_string(format.totalSamples));
    logger->Write("");

    return ExitStatusSuccess;
}

void Program::PrintAnalysisResults(MediaFile* file)
{
    PrintSectionHeader("Analysis Results");
    
    if (file->IsUpscaled())
    {
        logger->Write("File appears to be an upscale conversion");
    }
    else
    {
        logger->Write("File appears to be a natural bit-depth");
    }
}

std::shared_ptr<MediaFile> Program::OpenFile(std::string fileName)
{
    std::shared_ptr<MediaFile> inputFile;
    MediaFileType type = GetType(fileName);
    switch (type)
    {
        case MediaFileType::Wave:
            inputFile = std::make_shared<WaveFile>(fileName, logger);
            break;
        case MediaFileType::Flac:
            inputFile = std::make_shared<FlacFile>(fileName, logger);
            break;
        case MediaFileType::Unsupported:
            logger->Write("Unsupported file type", Logging::LogLevel::Error);
            return nullptr;
    }

    if (inputFile->Exists())
    {
        PrintSectionHeader("File");
        PrintField("Filename", inputFile->FileName());
        logger->Write("");
    }
    else
    {
        std::stringstream error;
        error << inputFileParam->Value() << " does not exist!";
        logger->Write(error.str(), Logging::LogLevel::Error);
        return nullptr;
    }

    try
    {
        inputFile->Open();
    }
    catch (const MediaFormatError& error)
    {
        logger->Write(error.what(), Logging::LogLevel::Error);
        return nullptr;
    }
    
    return inputFile;
}

MediaFileType GetType(std::string fileName)
{
    std::filesystem::path path = std::filesystem::path{ fileName };
    std::string ext = path.extension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
    
    if (ext == ".WAV")
        return MediaFileType::Wave;
    else if (ext == ".FLAC")
        return MediaFileType::Flac;
    return MediaFileType::Unsupported;
}