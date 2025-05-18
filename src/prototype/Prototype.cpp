// Prototype.cpp - The prototype version of AudioResolutionAnalyzer.
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
// See the License for the specific language governing permissionsand
// limitations under the License.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "LibCppBinary.h"
#include "CmdLine.h"

// Represents the fields that make up the RIFF chunk descriptor in a wave file.
// This should make up the first 12 bytes of the file.
struct ChunkDescriptor
{
    // Should be set to the characters 'RIFF' in a valid wave file.
    Binary::StringField id{ 4 };

    // The size of the rest of the file - 8 bytes (the id and size).
    Binary::UInt32Field size{ 0 };

    // Should be set to the characters 'WAVE' in a valid wave file.
    Binary::StringField format{ 4 };
};

// Represents the header for each RIFF subchunk in the WAVE file.
struct SubChunkHeader
{
    // Common values include 'fmt ', 'info', and 'data'.
    Binary::StringField id{ 4 };

    // Indicates the size of the rest of the subchunk after this header.
    Binary::UInt32Field size{ 0 };
};

// Represents the data portion of the 'fmt ' subchunk after the header, which
// is used to tell the audio client what the format of the audio data is.
struct FormatInfo
{
    Binary::UInt16Field audioFormat{ 0 };
    Binary::UInt16Field channels{ 0 };
    Binary::UInt32Field sampleRate{ 0 };
    Binary::UInt32Field byteRate{ 0 };
    Binary::UInt16Field blockAlign{ 0 };
    Binary::UInt16Field bitsPerSample{ 0 };
};

// Provides the pointers to the command line parameter values.
struct CmdLineParams
{
    std::shared_ptr<CmdLine::ProgParam> progParam;
    std::shared_ptr<CmdLine::PosParam> inputFileParam;
    std::shared_ptr<CmdLine::PosParam> outputFileParam;
    std::shared_ptr<CmdLine::Option> analyzeOption;
    std::shared_ptr<CmdLine::Option> to8BitOption;
    std::shared_ptr<CmdLine::Option> to16BitOption;
    std::shared_ptr<CmdLine::Option> to24BitOption;
};

// Defines the information needed to process a wave file according to the
// command line arguments.
struct WaveProcessingInfo
{
    CmdLineParams params;
    std::shared_ptr<Binary::RawFileStream> inputFile;
    std::shared_ptr<Binary::RawFileStream> outputFile;
    ChunkDescriptor originalDescriptor;
    ChunkDescriptor newDescriptor;
    SubChunkHeader originalDataHeader;
    SubChunkHeader newDataHeader;
    SubChunkHeader formatHeader;
    FormatInfo originalFormat;
    FormatInfo newFormat;
    std::vector<std::shared_ptr<Binary::DataField>> otherFields;
};

// Reads the RIFF chunk descriptor from the specified binary file.
void ReadDescriptor(WaveProcessingInfo& info)
{
    info.inputFile->Read(&info.originalDescriptor.id);
    info.inputFile->Read(&info.originalDescriptor.size);
    info.inputFile->Read(&info.originalDescriptor.format);
}

// Writes the RIFF chunk descriptor to the specified binary file.
void WriteDescriptor(WaveProcessingInfo& info)
{
    info.outputFile->Write(&info.newDescriptor.id);
    info.outputFile->Write(&info.newDescriptor.size);
    info.outputFile->Write(&info.newDescriptor.format);
}

// Reads a RIFF sub-chunk header from the specified binary file.
SubChunkHeader ReadSubChunkHeader(WaveProcessingInfo& info)
{
    SubChunkHeader header;
    info.inputFile->Read(&header.id);
    info.inputFile->Read(&header.size);
    return header;
}

// Writes a RIFF sub-chunk header to the specified binary file.
void WriteSubChunkHeader(WaveProcessingInfo& info, SubChunkHeader& header)
{
    info.outputFile->Write(&header.id);
    info.outputFile->Write(&header.size);
}

// Reads the format info from the format subchunk from the specified binary 
// file.
void ReadFormatInfo(WaveProcessingInfo& info)
{
    info.inputFile->Read(&info.originalFormat.audioFormat);
    info.inputFile->Read(&info.originalFormat.channels);
    info.inputFile->Read(&info.originalFormat.sampleRate);
    info.inputFile->Read(&info.originalFormat.byteRate);
    info.inputFile->Read(&info.originalFormat.blockAlign);
    info.inputFile->Read(&info.originalFormat.bitsPerSample);
}

// Writes the format info to the format subchunk to the specified binary
// file.
void WriteFormatInfo(WaveProcessingInfo& info)
{
    info.outputFile->Write(&info.newFormat.audioFormat);
    info.outputFile->Write(&info.newFormat.channels);
    info.outputFile->Write(&info.newFormat.sampleRate);
    info.outputFile->Write(&info.newFormat.byteRate);
    info.outputFile->Write(&info.newFormat.blockAlign);
    info.outputFile->Write(&info.newFormat.bitsPerSample);
}

// Prints the fields of the specified RIFF chunk descriptor.
void PrintDescriptor(ChunkDescriptor& descriptor)
{
    std::cout << "Chunk Descriptor" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Chunk ID     : " << descriptor.id.ToString() << std::endl;
    std::cout << "Chunk Size   : " << descriptor.size.ToString() << std::endl;
    std::cout << "Format       : " << descriptor.format.ToString() << std::endl;
    std::cout << std::endl;
}

// Prints the fields of the specified format information.
void PrintFormatInfo(FormatInfo& info)
{
    std::cout << "Format Info" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Audio Format : " << info.audioFormat.ToString() << std::endl;
    std::cout << "Channels     : " << info.channels.ToString() << std::endl;
    std::cout << "Sample Rate  : " << info.sampleRate.ToString() << std::endl;
    std::cout << "Byte Rate    : " << info.byteRate.ToString() << std::endl;
    std::cout << "Block Align  : " << info.blockAlign.ToString() << std::endl;
    std::cout << "Bits / Sample: " << info.bitsPerSample.ToString() << std::endl;
}

// Defines all the command line parameters used to parse the command line args.
CmdLineParams DefineCmdLineParams()
{
    CmdLineParams params;

    CmdLine::ProgParam::Definition progDef;
    progDef.name = "convbitdepthproto";
    progDef.description = "converts WAV files to different bit depths";
    params.progParam = std::make_shared<CmdLine::ProgParam>(progDef);

    CmdLine::PosParam::Definition inputFileDef;
    inputFileDef.name = "input-file";
    inputFileDef.description = "The file to use as input for the conversion";
    inputFileDef.isMandatory = true;
    params.inputFileParam = std::make_shared<CmdLine::PosParam>(inputFileDef);

    CmdLine::PosParam::Definition outputFileDef;
    outputFileDef.name = "output-file";
    outputFileDef.description = "The file to write the converted data to";
    outputFileDef.isMandatory = false;
    params.outputFileParam = std::make_shared<CmdLine::PosParam>(
        outputFileDef);

    CmdLine::Option::Definition analyzeDef;
    analyzeDef.shortName = 'a';
    analyzeDef.longName = "analyze";
    analyzeDef.description = "determines if the specified file was upscaled";
    params.analyzeOption = std::make_shared<CmdLine::Option>(analyzeDef);

    CmdLine::Option::Definition to8BitDef;
    to8BitDef.shortName = 'e';
    to8BitDef.longName = "8";
    to8BitDef.description = "converts the file to 8-bit audio";
    params.to8BitOption = std::make_shared<CmdLine::Option>(to8BitDef);

    CmdLine::Option::Definition to16BitDef;
    to16BitDef.shortName = 's';
    to16BitDef.longName = "16";
    to16BitDef.description = "converts the file to 16-bit audio";
    params.to16BitOption = std::make_shared<CmdLine::Option>(to16BitDef);

    CmdLine::Option::Definition to24BitDef;
    to24BitDef.shortName = 't';
    to24BitDef.longName = "24";
    to24BitDef.description = "converts the file to 24-bit audio";
    params.to24BitOption = std::make_shared<CmdLine::Option>(to24BitDef);

    return params;
}

// Parses the specified command line arguments, storing the parsed information
// in the specified command line parameters.
bool ParseCmdLineArgs(int argc, char** argv, CmdLineParams& params)
{
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; i++)
    {
        arguments.push_back(std::string(argv[i]));
    }

    CmdLine::Parser parser{ params.progParam.get(), arguments };
    parser.Add(params.inputFileParam.get());
    parser.Add(params.outputFileParam.get());
    parser.Add(params.analyzeOption.get());
    parser.Add(params.to8BitOption.get());
    parser.Add(params.to16BitOption.get());
    parser.Add(params.to24BitOption.get());
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

// Determines whether the RIFF chunk descriptor represents a valid wave file.
bool FileIsWaveFile(ChunkDescriptor descriptor)
{
    if (descriptor.id.ToString() != "RIFF" 
        || descriptor.format.ToString() != "WAVE")
    {
        std::cerr << "FATAL ERROR: File does not appear to be in WAV format.";
        std::cerr << "Exiting..." << std::endl;
        return false;
    }

    return true;
}

// Updates the specified format info according to which conversion was
// specified at the command line.
void UpdateFormatInfo(WaveProcessingInfo& info)
{
    constexpr int bitsPerByte{ 8 };

    // Copy the existing format info into the new format info so the new
    // info starts with the same data as the original.
    info.newFormat.audioFormat.SetValue(info.originalFormat.audioFormat.Value());
    info.newFormat.channels.SetValue(info.originalFormat.channels.Value());
    info.newFormat.sampleRate.SetValue(info.originalFormat.sampleRate.Value());
    info.newFormat.blockAlign.SetValue(info.originalFormat.blockAlign.Value());
    info.newFormat.byteRate.SetValue(info.originalFormat.byteRate.Value());
    info.newFormat.bitsPerSample.SetValue(info.originalFormat.bitsPerSample.Value());

    // Determine the number of bits-per-sample for the specified conversion
    // as it is an important input to the calucations for the new info.
    int bitsPerSample = 16;
    if (info.params.to8BitOption->IsSpecified())
        bitsPerSample = 8;
    else if (info.params.to16BitOption->IsSpecified())
        bitsPerSample = 16;
    else if (info.params.to24BitOption->IsSpecified())
        bitsPerSample = 24;

    // We need to determine the number of bytes per single-channel sample to
    // calculate the  block align.
    int bytesPerSample = bitsPerSample / bitsPerByte;

    // Block align tells us how many bytes there are in a sample frame, which
    // inludes the sample values for each channel.
    info.newFormat.blockAlign.SetValue(bytesPerSample * info.originalFormat.channels.Value());

    // Byte rate is the number of bytes per second give the sample rate.
    info.newFormat.byteRate.SetValue(info.newFormat.blockAlign.Value() 
        * info.newFormat.sampleRate.Value());

    info.newFormat.bitsPerSample.SetValue(bitsPerSample);
}

// Performs the conversion of 16-bit samples to 24-bit samples, reading the
// 16-bit samples from the specified input file and writing the 24-bit samples
// to the specified output file.
void Convert16To24Bit(WaveProcessingInfo& info)
{
    // 16-bit samples are 2-bytes wide, so if we divide the total data subchunk
    // size by 2, we can determine the number of samples.
    long numberOfSamples = info.originalDataHeader.size.Value() / 2;

    // 24-bit samples are 3-bytes wide, so multiplying the number of samples by
    // 3 gives us the new size of the data subchunk. 
    long newDataSize = numberOfSamples * 3;

    // Determine by how much the file size will increase after the conversion
    // as we will need to update the RIFF chunk descriptor with the new size.
    long sizeIncrease = newDataSize - info.originalDataHeader.size.Value();

    // Update the conversion info with the new data size so we can accurately
    // write the data to the converted file.
    info.newDataHeader.id.SetValue(info.originalDataHeader.id.Value());
    info.newDataHeader.size.SetValue(newDataSize);

    // The new RIFF chunk descriptor will stay the same as the origial except
    // for the size increase.
    info.newDescriptor.id.SetValue(info.originalDescriptor.id.Value());
    info.newDescriptor.size.SetValue(info.originalDescriptor.size.Value() + sizeIncrease);
    info.newDescriptor.format.SetValue(info.originalDescriptor.format.Value());

    // Start writing out the modified RIFF chunk descriptor and subchunks to 
    // the converted file as we have everything we need to start writing those.
    WriteDescriptor(info);
    WriteSubChunkHeader(info, info.formatHeader);
    WriteFormatInfo(info);

    // Writes the additional subchunk fields that this program is not concerned
    // about. This is things like the fields for the info subchunk. It copies 
    // these as-is to the new file.
    for (auto field : info.otherFields)
        info.outputFile->Write(field.get());

    // After writing all the other subchunk fields, the data subchunk should be
    // written last. 
    info.outputFile->Write(&info.newDataHeader.id);
    info.outputFile->Write(&info.newDataHeader.size);

    // We determine the number of bytes remaining so we don't read past the end
    // of the file.
    long bytesRemaining = info.originalDataHeader.size.Value();

    // We need to know bytes per sample so we can see how many bytes are
    // remaining after we convert each sample.
    int bytesPerSample = info.originalFormat.bitsPerSample.Value() / 8;

    // Now convert each sample to 24-bit and write out the 24-bit samples to 
    // finish the conversion.
    while (bytesRemaining > 0)
    {
        // Read the original sample into a 16-bit signed integer field to
        // prepare for conversion.
        Binary::Int16Field sample{ 0 };
        info.inputFile->Read(&sample);

        // Copy the value from the 16-bit integer into the 24-bit integer,
        // shifting the bits to the left by 8. This conversion essentially
        // "scales up" the value by 2^8 and zero pads the least significant
        // byte. I believe this is what is happening when you send 16-bit
        // audio into a 24-bit DAC or DSP.
        //
        // Each sample value represents the amplitude or "height" of the
        // waveform at a specific point in time. The fact that this bit 
        // shift works proves that different bit-depths operate at different
        // scales. In other words, to represent the same amplitude from a
        // 16-bit value in 24-bits, you need to multiply it by 2^8 (or shift
        // the bits by 8) to achieve the same amplitude at the larger scale
        // afforded by 24-bits.
        Binary::Int24Field upscaledSample{ 0 };
        upscaledSample.SetValue(sample.Value() << 8);

        info.outputFile->Write(&upscaledSample);
        bytesRemaining -= bytesPerSample;
    }
}

void ProcessFormatSubchunk(SubChunkHeader& header, WaveProcessingInfo& info)
{
    info.formatHeader.id.SetValue(header.id.Value());
    info.formatHeader.size.SetValue(header.size.Value());
    //info.originalFormat = ReadFormatInfo(info);
    ReadFormatInfo(info);
    PrintFormatInfo(info.originalFormat);
    //info.newFormat = UpdateFormatInfo(info);
    UpdateFormatInfo(info);
}

int ProcessDataSubchunk(SubChunkHeader& header, WaveProcessingInfo& info)
{
    info.originalDataHeader.id.SetValue(header.id.Value());
    info.originalDataHeader.size.SetValue(header.size.Value());
    std::cout << std::endl;

    if (info.params.to8BitOption->IsSpecified())
    {
        if (info.originalFormat.bitsPerSample.Value() == 8)
        {
            std::cerr << "Existing file already 8-bits" << std::endl;
            return 1;
        }
        else
        {
            std::cerr << "Conversion not available for this file";
            std::cerr << std::endl;
            return 1;
        }
    }
    else if (info.params.to16BitOption->IsSpecified())
    {
        if (info.originalFormat.bitsPerSample.Value() == 16)
        {
            std::cerr << "Existing file already 16-bits" << std::endl;
            return 1;
        }
        else
        {
            std::cerr << "Conversion not available for this file";
            std::cerr << std::endl;
            return 1;
        }
    }
    else if (info.params.to24BitOption->IsSpecified())
    {
        std::cout << "Converting wave file to 24-bit" << std::endl;

        if (info.originalFormat.bitsPerSample.Value() == 24)
        {
            std::cerr << "Existing file already 24-bits" << std::endl;
            return 1;
        }
        else if (info.originalFormat.bitsPerSample.Value() == 16)
        {
            Convert16To24Bit(info);
            return 0;
        }
        else
        {
            std::cerr << "Conversion not available for this file";
            std::cerr << std::endl;
            return 1;
        }
    }

    return 0;
}

void ProcessOtherSubchunk(SubChunkHeader& header, WaveProcessingInfo& info)
{
    auto subChunkID = std::make_shared<Binary::StringField>(4);
    auto subChunkSize = std::make_shared<Binary::UInt32Field>(0);
    auto subChunkData = std::make_shared<Binary::RawField>(header.size.Value());

    subChunkID->SetValue(header.id.Value());
    subChunkSize->SetValue(header.size.Value());
    info.inputFile->Read(subChunkData.get());

    info.otherFields.push_back(subChunkID);
    info.otherFields.push_back(subChunkSize);
    info.otherFields.push_back(subChunkData);
}

int ProcessWaveFile(WaveProcessingInfo& info)
{
    ReadDescriptor(info);
    if (!FileIsWaveFile(info.originalDescriptor))
        return 1;
    PrintDescriptor(info.originalDescriptor);
    
    bool dataFound = false;
    while (!dataFound)
    {
        SubChunkHeader header = ReadSubChunkHeader(info);

        if (header.id.ToString() == "fmt ")
        {
            ProcessFormatSubchunk(header, info);
        }
        else if (header.id.ToString() == "data")
        {
            dataFound = true;
            return ProcessDataSubchunk(header, info);
        }
        else
        {
            ProcessOtherSubchunk(header, info);
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    std::cout << "AudioResolutionAnalyzer Prototype" << std::endl;
    std::cout << "Copyright (C) 2024 Stephen Bonar" << std::endl << std::endl;

    WaveProcessingInfo info;
    info.params = DefineCmdLineParams();
    if (!ParseCmdLineArgs(argc, argv, info.params))
    {
        return 1;
    }

    auto fileReadStream = 
        std::make_shared<Binary::RawFileStream>(
            info.params.inputFileParam->Value());
    auto fileWriteStream =
        std::make_shared<Binary::RawFileStream>(
            info.params.outputFileParam->Value());
    fileReadStream->Open(Binary::FileMode::Read);
    fileWriteStream->Open(Binary::FileMode::Write);

    info.inputFile = fileReadStream;
    info.outputFile = fileWriteStream;
    return ProcessWaveFile(info);
}
