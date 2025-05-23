// AnalysisThread.h - Declares the AnalysisThread class.
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

#ifndef ANALYSIS_THREAD_H
#define ANALYSIS_THREAD_H

#include <vector>
#include <memory>
#include <filesystem>
#include <wx/wx.h>
#include "MediaFile.h"

class AnalysisThread : public wxThread
{
public:
    static constexpr int StatusUpdateID{ 10000 };
    static constexpr int StatusCompleteID{ 10001 };

    AnalysisThread(wxFrame* parent, 
                   std::vector<std::shared_ptr<MediaFile>>& fileList)
        : parent{ parent }, fileList{ fileList } { }

    ExitCode Entry() override;
private:
    wxFrame* parent;
    std::vector<std::shared_ptr<MediaFile>>& fileList;
};

#endif