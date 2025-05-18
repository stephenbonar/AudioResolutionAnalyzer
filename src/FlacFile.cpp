// FlacFile.cpp - Defines the FlacFile class methods.
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

#include "FlacFile.h"

FlacFile::~FlacFile()
{
    if (file != nullptr)
        fclose(file);
}

void FlacFile::Open()
{
    file = fopen(fileName.c_str(), "rb");
}

void FlacFile::Analyze(bool dumpSamples)
{
    // Start by assuming the file is an upscale conversion; the analysis will
    // disprove it if it finds any non-zero least significant bytes.
    isUpscaled = true;

    this->dumpSamples = dumpSamples;

    // Calls the init method from FLAC::Decoder::File, which opens the file
    // for reading and decoding.
    FLAC__StreamDecoderInitStatus initStatus = init(file);

    if (initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK) 
    {
        // Calling this method from FLAC::Decoder::Stream, base of 
        // FLAC::Decoder::File, starts decoding the FLAC until the end of the
        // stream. Each decoded frame can be retrieved using the callback
        // methods. NOTE: We use the write_callback method to retrieve and
        // analyze the frame buffers even though it's really meant for writing.
		if (!process_until_end_of_stream())
        {
            std::stringstream streamerror;
            streamerror << "FLAC stream error: ";
            streamerror << get_state().resolved_as_cstring(*this);
            logger->Write(streamerror.str(), Logging::LogLevel::Error);
        }
	}
    else
    {
        logger->Write(
            "Unable to initialize FLAC decoder", 
            Logging::LogLevel::Error);
    }
}

void FlacFile::Convert(
    std::string outputFileName, 
    BitDepth depth, 
    ConversionMethod method)
{
    logger->Write(
        "Conversion not yet implemented for FLAC", 
        Logging::LogLevel::Error);
}

::FLAC__StreamDecoderWriteStatus FlacFile::write_callback(
    const ::FLAC__Frame *frame, 
    const FLAC__int32 * const buffer[])
{
    const FLAC__uint32 total_size = 
        (FLAC__uint32)
        (format.totalSamples * format.channels * (format.bitsPerSample / 8));

	format.channels = FlacFile::get_channels();
	format.bitsPerSample = FlacFile::get_bits_per_sample();

	if (format.totalSamples == 0) 
    {
        logger->Write(
            "Flac STREAMINFO must include total samples", 
            Logging::LogLevel::Error);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

    format.blockSize = frame->header.blocksize;
	for (size_t frameIndex = 0; frameIndex < format.blockSize; frameIndex++)
    {
        for (
            int channelIndex = 0; 
            channelIndex < format.channels; 
            channelIndex++)
        {
            if (format.bitsPerSample == 32)
            {
                ProcessNext<Binary::Int32Field>(
                    buffer[channelIndex][frameIndex]);
            }
            else if (format.bitsPerSample == 24)
            {
                ProcessNext<Binary::Int24Field>(
                    buffer[channelIndex][frameIndex]);
            }
            else if (format.bitsPerSample == 16)
            {
                ProcessNext<Binary::Int16Field>(
                    buffer[channelIndex][frameIndex]);
            }
            else if (format.bitsPerSample == 8)
            {
                ProcessNext<Binary::UInt8Field>(
                    buffer[channelIndex][frameIndex]);
            }
        }
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacFile::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		format.totalSamples = metadata->data.stream_info.total_samples;
		format.sampleRate = metadata->data.stream_info.sample_rate;
		format.channels = metadata->data.stream_info.channels;
		format.bitsPerSample = metadata->data.stream_info.bits_per_sample;
	}
}
	
void FlacFile::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
    logger->Write("Flac decode error encountered!", Logging::LogLevel::Error);
}