// FlacFile.h - Declares the FlacFile class.
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

#ifndef FLAC_FILE_H
#define FLAC_FILE_H

#include <stdio.h>
#include <string>
#include <filesystem>
#include <memory>
#include <sstream>
#include "LibCppBinary.h"
#include "LibCppLogging.h"
#include "MediaFile.h"
#include "SampleDumper.h"
#include "FLAC++/decoder.h"
#include "FlacFormat.h"

class FlacFile : public MediaFile, public FLAC::Decoder::File
{
public:
    FlacFile(std::string fileName, std::shared_ptr<Logging::Logger> logger) :
        FLAC::Decoder::File(),
        fileName{ fileName }, 
        isUpscaled{ false },
        logger{ logger }
        {}

    ~FlacFile();

    std::string FileName() const override { return fileName; }

    int BitsPerSample() const override { return format.bitsPerSample; }

    long SampleRate() const override { return format.sampleRate; }

    FlacFormat Format() const { return format; }

    bool IsOpen() const override { return file != nullptr; }

    void Open() override;

    void Analyze(bool dumpSamples) override;

    void Convert(
        std::string outputFileName, 
        BitDepth depth, 
        ConversionMethod method) override;

    bool IsUpscaled() const override { return isUpscaled; }
protected:
    ::FLAC__StreamDecoderWriteStatus write_callback(
        const ::FLAC__Frame *frame, 
        const FLAC__int32 * const buffer[]) override;

	void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;

	void error_callback(::FLAC__StreamDecoderErrorStatus status) override;
private:
    std::string fileName;
    FILE* file;
    bool isUpscaled;
    FlacFormat format;
    std::shared_ptr<Logging::Logger> logger;
    std::shared_ptr<SampleDumper> dumper;
    bool dumpSamples = false;

    template <typename T>
    void ProcessNext(FLAC__int32 sampleValue)
    {
        T sample;
        sample.SetValue(sampleValue);

        if (dumpSamples)
        {
            if (dumper == nullptr)
                dumper = std::make_shared<SampleDumper>(fileName);

            dumper->Dump(&sample);
        }

        // Perform a bitwise and against the bitmask 0xFF to select the
        // bits in the least significant byte. If even one is of the 
        // least significant bytes is non-zero, the file is not likely
        // to be an upscale conversion.
        if ((sample.Value() & 0xFF) != 0)
            isUpscaled = false;
    }
};

#endif