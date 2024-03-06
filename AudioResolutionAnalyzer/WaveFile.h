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
#include <iostream>
#include <memory>
#include <filesystem>
#include "BinData.h"
#include "RiffChunkHeader.h"
#include "RiffSubChunkHeader.h"
#include "WaveFormat.h"
#include "BitDepth.h"
#include "ConversionMethod.h"

class WaveFile
{
public:
    WaveFile(std::string fileName);

    void Open();

    bool Exists() { return std::filesystem::exists(fileName); }

    RiffChunkHeader GetChunkHeader() { return chunkHeader; }

    WaveFormat GetFormat() { return format; }

    void Analyze();

    void Convert(
        std::string outputFileName, 
        BitDepth depth, 
        ConversionMethod method);

    bool IsUpscaleConversion() { return isUpscaleConversion; }
private:
    bool isUpscaleConversion;
    std::string fileName;
    RiffChunkHeader chunkHeader;
    RiffSubChunkHeader formatHeader;
    RiffSubChunkHeader dataHeader;
    WaveFormat format;
    std::vector<std::shared_ptr<BinData::Field>> otherFields;
    std::shared_ptr<BinData::StandardFileStream> readStream;
    std::shared_ptr<BinData::StandardFileStream> writeStream;

    RiffChunkHeader ReadChunkHeader();

    RiffSubChunkHeader ReadSubChunkHeader();

    WaveFormat ReadWaveFormat();

    void WriteChunkHeader(RiffChunkHeader header);

    void WriteSubChunkHeader(RiffSubChunkHeader header);

    void WriteFormatInfo(WaveFormat format);

    template <typename OriginalField>
    bool ConvertNextSample(BitDepth depth, ConversionMethod method)
    {
        switch (method)
        {
            case ConversionMethod::DirectCopy:
                switch (depth)
                {
                    case BitDepth::UInt8:
                        return DirectCopyNextSample<OriginalField, BinData::UInt8Field>();
                    case BitDepth::Int16:
                        return DirectCopyNextSample<OriginalField, BinData::Int16Field>();
                    case BitDepth::Int24:
                        return DirectCopyNextSample<OriginalField, BinData::Int24Field>();
                    case BitDepth::Int32:
                        return DirectCopyNextSample<OriginalField, BinData::Int32Field>();
                    default:
                        return false;
                }
            case ConversionMethod::LinearScaling:
                switch (depth)
                {
                    case BitDepth::UInt8:
                        return ScaleNextSampleLinear<OriginalField, BinData::UInt8Field>();
                    case BitDepth::Int16:
                        return ScaleNextSampleLinear<OriginalField, BinData::Int16Field>();
                    case BitDepth::Int24:
                        return ScaleNextSampleLinear<OriginalField, BinData::Int24Field>();
                    case BitDepth::Int32:
                        return ScaleNextSampleLinear<OriginalField, BinData::Int32Field>();
                    default:
                        return false;
                }
            default:
                return false;  
        }   
    }

    template<typename OriginalField, typename NewField>
    bool DirectCopyNextSample()
    {
        // Read the original sample to prepare it for conversion.
        OriginalField sample{ 0 };
        readStream->Read(&sample);

        NewField newSample{ 0 };
        if (newSample.Size() < sample.Size())
        {
            std::cerr << 
                "Cannot do a direct copy conversion to smaller bit depth";
            std::cerr << std::endl;
            return false;
        }
        newSample.SetValue(sample.Value());

        writeStream->Write(&newSample);
        return true;
    }

    template <typename OriginalField, typename NewField>
    bool ScaleNextSampleLinear()
    {
        // Read the original sample to prepare it for conversion.
        OriginalField sample{ 0 };
        readStream->Read(&sample);

        // Allocate a dummy sample so we can check the field size of the new
        // sample field type vs the original. TODO: see if we can do this
        // without allocating a dummy.
        NewField dummySample{ 0 };

        // Determine if a conversion is actually needed so we can abort the
        // operation if it is not necessary.
        if (sample.Size() == dummySample.Size())
        {
            int currentBitDepth = sample.Size() * 8;
            std::cerr << "ERROR: file already " << currentBitDepth << " bits.";
            std::cerr << std::endl;
            return false;
        }
        
        int newFieldDifference = dummySample.Size() - sample.Size();
        if (newFieldDifference > 0)
            UpscaleLinear<OriginalField, NewField>(sample);
        else
            DownscaleLinear<OriginalField, NewField>(sample);

        return true;
    }

    template <typename OriginalField, typename NewField>
    void DownscaleLinear(OriginalField& sample)
    {
        // Initialize a sample field with the new field type to store the
        // downscaled sample in. 
        NewField downscaledSample{ 0 };

        // Determine how much we decreased so we know how many bits to
        // shift the original sample to the right by.
        int byteDecrease = sample.Size() - downscaledSample.Size();
        int bitShift = byteDecrease * 8;

        // Determine if the new sample will be 8 bits as there are additional
        // transformations to perform in that case.
        bool is8Bit = downscaledSample.Size() == 1;

        // Copy the value from the original sample field into the downscaled
        // sample field, shifting the bits to the right. This conversion 
        // essentially "scales down" the value by 2^bitshift. For instance, 
        // when downscaling from 24-bits to 16-bits, the 24-bit value is 
        // shifted right by 8.
        //
        // Each sample value represents the amplitude or "height" of the
        // waveform at a specific point in time. The fact that this bit 
        // shift works proves that different bit-depths operate at different
        // scales. In other words, to represent the same amplitude from a
        // 24-bit value in 16-bits, you need to divide it by 2^8 (or shift
        // the bits right by 8) to achieve the same amplitude at the smaller
        // scale limited by 16-bits.
        //
        // When downscaling an 8-bit sample, we need to toggle the sign bit
        // of the signed value before we can put it in the unsigned 8-bit 
        // field. We accomplish this by doing an XOR of the sign bit which
        // would be 0x8000 for 16-bit, 0x800000 for 24-bit, etc. We can 
        // determine the correct XOR value by left shifting it by the
        // same number of bits we will right shift to downscale the value.
        if (is8Bit)
        {
            downscaledSample.SetValue(
                (sample.Value() ^ (0x80 << bitShift)) >> bitShift);
        }
        else
        {
            downscaledSample.SetValue(sample.Value() >> bitShift);
        }

        writeStream->Write(&downscaledSample);
    }

    template <typename OriginalField, typename NewField>
    void UpscaleLinear(OriginalField& sample)
    {
        // Initialize a sample field with the new field type to store the
        // upscaled sample in. 
        NewField upscaledSample{ 0 };

        // Determine how much we increased so we know how many bits to
        // shift the original sample to the left by.
        int byteIncrease = upscaledSample.Size() - sample.Size();
        int bitShift = byteIncrease * 8;

        // Determine if the current sample is 8 bits as there are additional
        // transformations to perform in that case.
        bool is8Bit = sample.Size() == 1;

        // Copy the value from the original sample field into the upscaled
        // sample field, shifting the bits to the left. This conversion 
        // essentially "scales up" the value by 2^bitshift and zero pads the
        // least significant bytes. For instance, when upscaling from 16-bits
        // to 24-bits, the 16-bit value is shifted left by 8. I believe this 
        // is what is happening when you send 16-bit audio into a 24-bit 
        // DAC or DSP.
        //
        // Each sample value represents the amplitude or "height" of the
        // waveform at a specific point in time. The fact that this bit 
        // shift works proves that different bit-depths operate at different
        // scales. In other words, to represent the same amplitude from a
        // 16-bit value in 24-bits, you need to multiply it by 2^8 (or shift
        // the bits by 8) to achieve the same amplitude at the larger scale
        // afforded by 24-bits.
        //
        // When upscaling an 8-bit sample, we need to subtract the 8-bit 
        // unsigned value (which can be anything from 0 - 255) from the 
        // midpoint of an unsigned 8-bit value (128 or 0x80) to convert it
        // to a signed value before scaling the value up. 0 - 127 would
        // become a negative number and 128 - 255 would become positive.
        if (is8Bit)
        {
            upscaledSample.SetValue(
               (sample.Value() - 0x80) << bitShift);
        }
        else
        {
            upscaledSample.SetValue(sample.Value() << bitShift);
        }

        writeStream->Write(&upscaledSample);
    }

    RiffChunkHeader GetNewChunkHeader(long sizeIncrease);

    WaveFormat GetNewWaveFormat(BitDepth depth);

    long CalculateNumberOfSamples();

    long CalculateNewDataSize(BitDepth depth, long numberOfSamples);

    template <typename T>
    void AnalyzeNextSample()
    {
        T sample{ 0 };
        readStream->Read(&sample);

        // Perform a bitwise and against the bitmask 0xFF to select the bits in
        // the least significant byte. If even one is of the least significant
        // bytes is non-zero, the file is not likely to be an upscale
        // conversion.
        if ((sample.Value() & 0xFF) != 0)
            isUpscaleConversion = false;
    }
};

#endif