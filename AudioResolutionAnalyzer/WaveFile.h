// WaveFile.h - Declares the WaveFile class.
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

#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include "BinData.h"
#include "RiffChunkHeader.h"
#include "RiffSubChunkHeader.h"
#include "WaveFormat.h"
#include "BitDepth.h"
#include "ConversionMethod.h"
#include "SampleConverter.h"
#include "MediaFile.h"
#include "Logging.h"
#include "SampleDumper.h"

class WaveFile : public MediaFile
{
public:
    static constexpr int WaveFormatPcm{ 0x1 };
    static constexpr int WaveFormatExtensible{ 0xFFFE };

    WaveFile(std::string fileName, std::shared_ptr<Logging::Logger> logger);

    int BitsPerSample() const override { return format.bitsPerSample.Value(); }

    long SampleRate() const override { return format.sampleRate.Value(); }

    void Open() override;

    RiffChunkHeader ChunkHeader() const { return chunkHeader; }

    WaveFormat Format() const { return format; }

    std::string FileName() const override { return fileName; }

    void Analyze(bool dumpSamples) override;

    void Convert(
        std::string outputFileName, 
        BitDepth depth, 
        ConversionMethod method) override;

    bool IsUpscaled() const override { return isUpscaled; }
private:
    bool isUpscaled;
    std::string fileName;
    RiffChunkHeader chunkHeader;
    RiffSubChunkHeader formatHeader;
    RiffSubChunkHeader dataHeader;
    WaveFormat format;
    std::vector<std::shared_ptr<BinData::Field>> otherFields;
    std::shared_ptr<BinData::StdFileStream> readStream;
    std::shared_ptr<BinData::StdFileStream> writeStream;
    std::shared_ptr<Logging::Logger> logger;
    std::shared_ptr<SampleDumper> sampleDumper;

    RiffChunkHeader ReadChunkHeader();

    RiffSubChunkHeader ReadSubChunkHeader();

    WaveFormat ReadWaveFormat();

    void WriteChunkHeader(RiffChunkHeader header);

    void WriteSubChunkHeader(RiffSubChunkHeader header);

    void WriteFormatInfo(WaveFormat format);

    template <typename T>
    bool ConvertNext(ConversionMethod method, BitDepth depth)
    {
        T sample { 0 };
        readStream->Read(&sample);
        auto converter = SampleConverter<T>(method, depth);
        std::shared_ptr<BinData::Field> newSample = converter.Convert(sample);

        if (newSample != nullptr)
        {
            writeStream->Write(newSample.get());
            return true;
        }
        else
        {
            return false;
        }
    }

    RiffChunkHeader GetNewChunkHeader(long sizeIncrease);

    WaveFormat GetNewWaveFormat(BitDepth depth);

    long CalculateNumberOfSamples();

    long CalculateNewDataSize(BitDepth depth, long numberOfSamples);

    template <typename T>
    void AnalyzeNextSample(bool dumpSamples)
    {
        T sample{ 0 };
        readStream->Read(&sample);

        if (dumpSamples)
        {
            sampleDumper->Dump(&sample);
        }
        
        // Perform a bitwise and against the bitmask 0xFF to select the bits in
        // the least significant byte. If even one is of the least significant
        // bytes is non-zero, the file is not likely to be an upscale
        // conversion.
        if ((sample.Value() & 0xFF) != 0)
            isUpscaled = false;
    }
};

#endif