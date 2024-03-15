#ifndef SAMPLE_CONVERTER_H
#define SAMPLE_CONVERTER_H

#include <memory>
#include "ConversionMethod.h"
#include "BitDepth.h"

template <typename T>
class SampleConverter
{
public:
    SampleConverter(ConversionMethod method, BitDepth newFormat)
    {
        this->method = method;
        this->newFormat = newFormat;
    }

    std::shared_ptr<BinData::Field> Convert(T& sample)
    {
        switch (method)
        {
            case ConversionMethod::DirectCopy:
                return ConvertDirectCopy(sample);
            case ConversionMethod::LinearScaling:
                return ConvertLinearScaling(sample);
            default:
                return nullptr;
        }
    }
private:
    ConversionMethod method;
    BitDepth newFormat;

    std::shared_ptr<BinData::Field> ConvertDirectCopy(T& sample)
    {
        switch (newFormat)
        {
            case BitDepth::UInt8:
                return DirectCopyTo<BinData::UInt8Field>(sample);
            case BitDepth::Int16:
                return DirectCopyTo<BinData::Int16Field>(sample);
            case BitDepth::Int24:
                return DirectCopyTo<BinData::Int24Field>(sample);
            case BitDepth::Int32:
                return DirectCopyTo<BinData::Int32Field>(sample);
            default:
                return nullptr;
        }
    }

    template <typename U>
    std::shared_ptr<BinData::Field> DirectCopyTo(T& sample)
    {
        auto newSample = std::make_shared<U>(0);
        if (newSample->Size() < sample.Size())
        {
            std::cerr << 
                "Cannot do a direct copy conversion to smaller bit depth";
            std::cerr << std::endl;
            return nullptr;
        }
        newSample->SetValue(sample.Value());
        return newSample;
    }

    std::shared_ptr<BinData::Field> ConvertLinearScaling(T& sample)
    {
        switch (newFormat)
        {
            case BitDepth::UInt8:
                return LinearScaleTo<BinData::UInt8Field>(sample);
            case BitDepth::Int16:
                return LinearScaleTo<BinData::Int16Field>(sample);
            case BitDepth::Int24:
                return LinearScaleTo<BinData::Int24Field>(sample);
            case BitDepth::Int32:
                return LinearScaleTo<BinData::Int32Field>(sample);
            default:
                return nullptr;
        }
    }

    template <typename U>
    std::shared_ptr<BinData::Field> LinearScaleTo(T& sample)
    {
        // Allocate a dummy sample so we can check the field size of the new
        // sample field type vs the original. TODO: see if we can do this
        // without allocating a dummy.
        U dummySample{ 0 };

        // Determine if a conversion is actually needed so we can abort the
        // operation if it is not necessary.
        if (sample.Size() == dummySample.Size())
        {
            int currentBitDepth = sample.Size() * 8;
            std::cerr << "ERROR: file already " << currentBitDepth << " bits.";
            std::cerr << std::endl;
            return nullptr;
        }
        
        int newFieldDifference = dummySample.Size() - sample.Size();
        if (newFieldDifference > 0)
            return LinearUpscaleTo<U>(sample);
        else
            return LinearDownscaleTo<U>(sample);
    }
    
    template <typename U>
    std::shared_ptr<BinData::Field> LinearUpscaleTo(T& sample)
    {
        // Initialize a sample field with the new field type to store the
        // upscaled sample in. 
        auto upscaledSample = std::make_shared<U>(0);

        // Determine how much we increased so we know how many bits to
        // shift the original sample to the left by.
        int byteIncrease = upscaledSample->Size() - sample.Size();
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
            upscaledSample->SetValue(
               (sample.Value() - 0x80) << bitShift);
        }
        else
        {
            upscaledSample->SetValue(sample.Value() << bitShift);
        }

        return upscaledSample;
    }

    template <typename U>
    std::shared_ptr<BinData::Field> LinearDownscaleTo(T& sample)
    {
        // Initialize a sample field with the new field type to store the
        // downscaled sample in. 
        auto downscaledSample = std::make_shared<U>(0);

        // Determine how much we decreased so we know how many bits to
        // shift the original sample to the right by.
        int byteDecrease = sample.Size() - downscaledSample->Size();
        int bitShift = byteDecrease * 8;

        // Determine if the new sample will be 8 bits as there are additional
        // transformations to perform in that case.
        bool is8Bit = downscaledSample->Size() == 1;

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
            downscaledSample->SetValue(
                (sample.Value() ^ (0x80 << bitShift)) >> bitShift);
        }
        else
        {
            downscaledSample->SetValue(sample.Value() >> bitShift);
        }

        return downscaledSample;
    }
};

#endif