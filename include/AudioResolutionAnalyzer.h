// AudioResolutionAnalyzer.h - Declares the AudioResolutionAnalyzer class.
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

#ifndef AUDIO_RESOLUTION_ANALYZER_H
#define AUDIO_RESOLUTION_ANALYZER_H

#include <sstream>
#include <wx/wx.h>
#include "MainWindow.h"
#include "Version.h"

class AudioResolutionAnalyzer : public wxApp
{
public:
    bool OnInit() override;
};

#endif