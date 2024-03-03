// ConversionMethod.h - Declares the ConversionMethod enum.
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

#ifndef CONVERSION_METHOD_H
#define CONVERSION_METHOD_H

/// @brief Represents all possible conversion methods between bit depths.
enum ConversionMethod
{
    /// @brief Directly copies the sample value.
    ///
    /// Directly copies the sample value from the smaller sized integer to the
    /// larger sized integer. This does not provide a valid conversion; the 
    /// reason this method is included is to prove that converting from one bit
    /// depth to another is not a simple copy of the same numeric value into an
    /// integer of a different size.
    CopyByValue,

    /// @brief Scales the sample value with an 8-bit left shift.
    ///
    /// Scales the sample value up by shifting the bits left by 8 before
    /// storing the value in the larger sized integer. This essentially
    /// maintains a 1:1 ratio when the value is transferred to the larger
    /// integer. This results in a valid conversion.
    Upscaling,

    /// @brief 
    UpscalingWithInterpolation
};

#endif