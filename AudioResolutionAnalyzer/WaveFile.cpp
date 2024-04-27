// WaveFile.cpp - Defines the WaveFile class.
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

#include "WaveFile.h"

WaveFile::WaveFile(
    std::string fileName, 
    std::shared_ptr<Logging::Logger> logger)
{
    this->fileName = fileName;
    this->logger = logger;
    this->isUpscaled = false;
    readStream = std::make_shared<BinData::StdFileStream>(fileName);
    sampleDumper = std::make_shared<SampleDumper>(fileName);
}

void WaveFile::Open()
{
    if (!Exists())
        logger->Write("File does not exist!", Logging::LogLevel::Error);

    if (!readStream->IsOpen())
    {
        readStream->Open(BinData::FileMode::Read);
        if (!readStream->IsOpen())
            logger->Write("Unable to open file", Logging::LogLevel::Error);
    }
        
    chunkHeader = ReadChunkHeader();

    bool dataFound = false;
    while (!dataFound)
    {
        RiffSubChunkHeader subChunkHeader = ReadSubChunkHeader();
        if (subChunkHeader.id.ToString() == "fmt ")
        {
            formatHeader = subChunkHeader;
            try
            {
                format = ReadWaveFormat();
            }
            catch (const MediaFormatError& error)
            {
                throw;
            } 
        }
        else if (subChunkHeader.id.ToString() == "data")
        {
            dataHeader = subChunkHeader;
            dataFound = true;
        }
        else
        {
            auto subChunkID = std::make_shared<BinData::StringField>(4);
            auto subChunkSize = std::make_shared<BinData::UInt32Field>(0);
            auto subChunkData = std::make_shared<BinData::RawField>(
                subChunkHeader.size.Value());

            subChunkID->SetData(subChunkHeader.id.ToString());
            subChunkSize->SetValue(subChunkHeader.size.Value());
            readStream->Read(subChunkData.get());

            otherFields.push_back(subChunkID);
            otherFields.push_back(subChunkSize);
            otherFields.push_back(subChunkData);
        }
    }
}

RiffChunkHeader WaveFile::ReadChunkHeader()
{
    RiffChunkHeader header;
    readStream->Read(&header.id);
    readStream->Read(&header.size);
    readStream->Read(&header.type);
    return header;
}

RiffSubChunkHeader WaveFile::ReadSubChunkHeader()
{
    RiffSubChunkHeader header;
    readStream->Read(&header.id);
    readStream->Read(&header.size);
    return header;
}

WaveFormat WaveFile::ReadWaveFormat()
{
    WaveFormat format;

    readStream->Read(&format.audioFormat);
    if (format.audioFormat.Value() == WaveFormatExtensible)
        throw MediaFormatError{ "Extensible WAVE format not yet supported" };
    else if (format.audioFormat.Value() != WaveFormatPcm)
        throw MediaFormatError{ "Non-PCM wave formats not supported" };

    readStream->Read(&format.channels);
    readStream->Read(&format.sampleRate);
    readStream->Read(&format.byteRate);
    readStream->Read(&format.blockAlign);
    readStream->Read(&format.bitsPerSample);

    return format;
}

void WaveFile::Analyze(bool dumpSamples)
{
    unsigned long bytesRemaining = dataHeader.size.Value();

    // Start by assuming the file is an upscale conversion; the analysis will
    // disprove it if it finds any non-zero least significant bytes.
    isUpscaled = true;
    
    int bytesPerSample = 0;
    switch (format.bitsPerSample.Value())
    {
        case 8:
            bytesPerSample = 1;
            break;
        case 16:
            bytesPerSample = 2;
            break;
        case 24:
            bytesPerSample = 3;
            break;
        case 32:
            bytesPerSample = 4;
            break;
    }
    if (bytesPerSample == 0)
        return;

    while (bytesRemaining > 0)
    {
        switch (format.bitsPerSample.Value())
        {
            case 8:
                AnalyzeNextSample<BinData::UInt8Field>(dumpSamples);
                break;
            case 16:
                AnalyzeNextSample<BinData::Int16Field>(dumpSamples);
                break;
            case 24:
                AnalyzeNextSample<BinData::Int24Field>(dumpSamples);
                break;
            case 32:
                AnalyzeNextSample<BinData::Int32Field>(dumpSamples);
                break;
        }
        
        bytesRemaining -= bytesPerSample;
    }
}

void WaveFile::Convert(
    std::string outputFileName, 
    BitDepth depth, 
    ConversionMethod method)
{
    // Open the file stream for writing so we can write the converted data. 
    writeStream = std::make_shared<BinData::StdFileStream>(outputFileName);
    if (!writeStream->IsOpen())
        writeStream->Open(BinData::FileMode::Write);

    // Calculate how the file will change after the conversion so we can set
    // the headers of the converted file to the appropriate values.
    long numberOfSamples = CalculateNumberOfSamples();
    long newDataSize = CalculateNewDataSize(depth, numberOfSamples);
    long sizeChange = newDataSize - dataHeader.size.Value();

    // Write the modified headers to the converted file to reflect the changes.
    WriteChunkHeader(GetNewChunkHeader(sizeChange));
    WriteSubChunkHeader(formatHeader);
    WriteFormatInfo(GetNewWaveFormat(depth));

    // Writes the additional subchunk fields that this program is not concerned
    // about. This is things like the fields for the info subchunk. It copies 
    // these as-is to the new file.
    for (auto field : otherFields)
        writeStream->Write(field.get());

    // After writing all the other subchunk fields, the data subchunk should be
    // written last. 
    RiffSubChunkHeader newDataHeader;
    newDataHeader.id = dataHeader.id;
    newDataHeader.size = newDataSize;
    WriteSubChunkHeader(newDataHeader);

    // We determine the number of bytes remaining in the existing file so we 
    // don't read past the end of the file.
    long bytesRemaining = dataHeader.size.Value();

    // We need to know bytes per sample so we can see how many bytes are
    // remaining after we convert each sample.
    int bytesPerSample = format.bitsPerSample.Value() / 8;

    // Now convert each sample and write out the converted samples to finish 
    // the conversion.
    while (bytesRemaining > 0)
    {
        bool success = false;
        switch (format.bitsPerSample.Value())
        {
            case 8:
                success = ConvertNext<BinData::UInt8Field>(method, depth);
                break;
            case 16:
                success = ConvertNext<BinData::Int16Field>(method, depth);
                break;
            case 24:
                success = ConvertNext<BinData::Int24Field>(method, depth);
                break;
            case 32:
                success = ConvertNext<BinData::Int32Field>(method, depth);
                break;
        }

        if (!success)
            break;
      
        bytesRemaining -= bytesPerSample;
    }
}

long WaveFile::CalculateNumberOfSamples()
{
    constexpr int bitsPerByte{ 8 };
    int bytesPerSample = format.bitsPerSample.Value() / bitsPerByte;
    return dataHeader.size.Value() / bytesPerSample;
}

long WaveFile::CalculateNewDataSize(BitDepth depth, long numberOfSamples)
{
    switch (depth)
    {
        case BitDepth::UInt8:
            return numberOfSamples * 1;
        case BitDepth::Int16:
            return numberOfSamples * 2;
        case BitDepth::Int24:
            return numberOfSamples * 3;
        case BitDepth::Int32:
            return numberOfSamples * 4;
        default:
            return 0;
    }
}

void WaveFile::WriteChunkHeader(RiffChunkHeader header)
{
    writeStream->Write(&header.id);
    writeStream->Write(&header.size);
    writeStream->Write(&header.type);
}

void WaveFile::WriteSubChunkHeader(RiffSubChunkHeader header)
{
    writeStream->Write(&header.id);
    writeStream->Write(&header.size);
}

void WaveFile::WriteFormatInfo(WaveFormat format)
{
    writeStream->Write(&format.audioFormat);
    writeStream->Write(&format.channels);
    writeStream->Write(&format.sampleRate);
    writeStream->Write(&format.byteRate);
    writeStream->Write(&format.blockAlign);
    writeStream->Write(&format.bitsPerSample);
}

RiffChunkHeader WaveFile::GetNewChunkHeader(long sizeIncrease)
{
    RiffChunkHeader newChunkHeader;
    newChunkHeader.id = chunkHeader.id;
    newChunkHeader.size = chunkHeader.size.Value() + sizeIncrease;
    newChunkHeader.type = chunkHeader.type;
    return newChunkHeader;
}

WaveFormat WaveFile::GetNewWaveFormat(BitDepth depth)
{
    constexpr int bitsPerByte{ 8 };

    // Copy the existing format info into the new format info so the new
    // info starts with the same data as the original.
    WaveFormat newFormat;
    newFormat.audioFormat = format.audioFormat;
    newFormat.channels = format.channels;
    newFormat.sampleRate = format.sampleRate;
    newFormat.blockAlign = format.blockAlign;
    newFormat.byteRate = format.byteRate;
    newFormat.bitsPerSample = format.bitsPerSample;

    // Determine the number of bits-per-sample for the specified conversion
    // as it is an important input to the calucations for the new info.
    int bitsPerSample = 16;
    if (depth == BitDepth::UInt8)
        bitsPerSample = 8;
    else if (depth == BitDepth::Int16)
        bitsPerSample = 16;
    else if (depth == BitDepth::Int24)
        bitsPerSample = 24;
    else if (depth == BitDepth::Int32)
        bitsPerSample = 32;

    // We need to determine the number of bytes per single-channel sample to
    // calculate the  block align.
    int bytesPerSample = bitsPerSample / bitsPerByte;

    // Block align tells us how many bytes there are in a sample frame, which
    // inludes the sample values for each channel.
    newFormat.blockAlign = bytesPerSample * format.channels.Value();

    // Byte rate is the number of bytes per second give the sample rate.
    newFormat.byteRate = newFormat.blockAlign.Value() 
        * newFormat.sampleRate.Value();

    newFormat.bitsPerSample = bitsPerSample;

    return newFormat;
}