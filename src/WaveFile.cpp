// WaveFile.cpp - Defines the WaveFile class.
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

#include "WaveFile.h"

WaveFile::WaveFile(
    std::string fileName, 
    std::shared_ptr<Logging::Logger> logger)
{
    this->fileName = fileName;
    this->logger = logger;
    this->isUpscaled = false;
    readStream = std::make_shared<Binary::RawFileStream>(fileName);
    //sampleDumper = std::make_shared<SampleDumper>(fileName);
}

void WaveFile::Open()
{
    if (!Exists())
        logger->Write("File does not exist!", Logging::LogLevel::Error);

    if (!readStream->IsOpen())
    {
        readStream->Open(Binary::FileMode::Read);
        if (!readStream->IsOpen())
            logger->Write("Unable to open file", Logging::LogLevel::Error);
    }
        
    //chunkHeader = ReadChunkHeader();
    readStream->Read(&riffChunkHeader);
    readStream->Read(&riffFileType);

    bool dataFound = false;

    while (!dataFound)
    {
        Binary::ChunkHeader subChunkHeader;
        //RiffSubChunkHeader subChunkHeader = ReadSubChunkHeader();
        readStream->Read(&subChunkHeader);

        if (subChunkHeader.id.ToString() == "fmt ")
        {
            formatHeader.id.SetValue(subChunkHeader.id.Value());
            formatHeader.dataSize.SetValue(subChunkHeader.dataSize.Value());

            try
            {
                readStream->Read(&format);
                //format = ReadWaveFormat();
            }
            catch (const MediaFormatError& error)
            {
                throw;
            } 
        }
        else if (subChunkHeader.id.ToString() == "data")
        {
            dataHeader.id.SetValue(subChunkHeader.id.Value());
            dataHeader.dataSize.SetValue(subChunkHeader.dataSize.Value());
            dataFound = true;
        }
        else
        {
            auto subChunkID = std::make_shared<Binary::StringField>(4);
            auto subChunkSize = std::make_shared<Binary::UInt32Field>(0);
            auto subChunkData = std::make_shared<Binary::RawField>(
                subChunkHeader.dataSize.Value());

            subChunkID->SetValue(subChunkHeader.id.Value());
            subChunkSize->SetValue(subChunkHeader.dataSize.Value());
            readStream->Read(subChunkData.get());

            otherFields.push_back(subChunkID);
            otherFields.push_back(subChunkSize);
            otherFields.push_back(subChunkData);
        }
    }
}

/*
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
*/

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
    unsigned long bytesRemaining = dataHeader.dataSize.Value();

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
                AnalyzeNextSample<Binary::UInt8Field>(dumpSamples);
                break;
            case 16:
                AnalyzeNextSample<Binary::Int16Field>(dumpSamples);
                break;
            case 24:
                AnalyzeNextSample<Binary::Int24Field>(dumpSamples);
                break;
            case 32:
                AnalyzeNextSample<Binary::Int32Field>(dumpSamples);
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
    writeStream = std::make_shared<Binary::RawFileStream>(outputFileName);
    if (!writeStream->IsOpen())
        writeStream->Open(Binary::FileMode::Write);

    // Calculate how the file will change after the conversion so we can set
    // the headers of the converted file to the appropriate values.
    long numberOfSamples = CalculateNumberOfSamples();
    long newDataSize = CalculateNewDataSize(depth, numberOfSamples);
    long sizeChange = newDataSize - dataHeader.dataSize.Value();

    // Write the modified headers to the converted file to reflect the changes.
    /*
    WriteChunkHeader(GetNewChunkHeader(sizeChange));
    WriteSubChunkHeader(formatHeader);
    WriteFormatInfo(GetNewWaveFormat(depth));
    */
    Binary::ChunkHeader newChunkHeader;
    newChunkHeader.id.SetValue(riffChunkHeader.id.Value());
    newChunkHeader.dataSize.SetValue(riffChunkHeader.dataSize.Value() + sizeChange);
    Binary::ChunkHeader formatSubChunk;
    formatSubChunk.id.SetValue("fmt ");
    formatSubChunk.dataSize.SetValue(16);
    WaveFormat newFormat = GetNewWaveFormat(depth);
    writeStream->Write(&newChunkHeader);
    writeStream->Write(&riffFileType);
    writeStream->Write(&formatSubChunk);
    writeStream->Write(&newFormat);


    // Writes the additional subchunk fields that this program is not concerned
    // about. This is things like the fields for the info subchunk. It copies 
    // these as-is to the new file.
    for (auto field : otherFields)
        writeStream->Write(field.get());

    // After writing all the other subchunk fields, the data subchunk should be
    // written last. 
    Binary::ChunkHeader newDataHeader;
    newDataHeader.id.SetValue(dataHeader.id.Value());
    newDataHeader.dataSize.SetValue(newDataSize);
    //WriteSubChunkHeader(newDataHeader);
    writeStream->Write(&newDataHeader);

    // We determine the number of bytes remaining in the existing file so we 
    // don't read past the end of the file.
    long bytesRemaining = dataHeader.dataSize.Value();

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
                success = ConvertNext<Binary::UInt8Field>(method, depth);
                break;
            case 16:
                success = ConvertNext<Binary::Int16Field>(method, depth);
                break;
            case 24:
                success = ConvertNext<Binary::Int24Field>(method, depth);
                break;
            case 32:
                success = ConvertNext<Binary::Int32Field>(method, depth);
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
    return dataHeader.dataSize.Value() / bytesPerSample;
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

/*
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
*/

/*
void WaveFile::WriteFormatInfo(WaveFormat format)
{
    writeStream->Write(&format.audioFormat);
    writeStream->Write(&format.channels);
    writeStream->Write(&format.sampleRate);
    writeStream->Write(&format.byteRate);
    writeStream->Write(&format.blockAlign);
    writeStream->Write(&format.bitsPerSample);
}*/

/*
RiffChunkHeader WaveFile::GetNewChunkHeader(long sizeIncrease)
{
    RiffChunkHeader newChunkHeader;
    newChunkHeader.id = chunkHeader.id;
    newChunkHeader.size = chunkHeader.size.Value() + sizeIncrease;
    newChunkHeader.type = chunkHeader.type;
    return newChunkHeader;
}
*/

WaveFormat WaveFile::GetNewWaveFormat(BitDepth depth)
{
    constexpr int bitsPerByte{ 8 };

    // Copy the existing format info into the new format info so the new
    // info starts with the same data as the original.
    WaveFormat newFormat;
    newFormat.audioFormat.SetValue(format.audioFormat.Value());
    newFormat.channels.SetValue(format.channels.Value());
    newFormat.sampleRate.SetValue(format.sampleRate.Value());
    newFormat.blockAlign.SetValue(format.blockAlign.Value());
    newFormat.byteRate.SetValue(format.byteRate.Value());
    newFormat.bitsPerSample.SetValue(format.bitsPerSample.Value());

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
    newFormat.blockAlign.SetValue(bytesPerSample * format.channels.Value());

    // Byte rate is the number of bytes per second give the sample rate.
    newFormat.byteRate.SetValue(newFormat.blockAlign.Value() 
        * newFormat.sampleRate.Value());

    newFormat.bitsPerSample.SetValue(bitsPerSample);

    return newFormat;
}