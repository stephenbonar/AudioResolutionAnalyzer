// AnalysisThread.cpp - Defines the AnalysisThread class methods.
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

#include "AnalysisThread.h"

wxThread::ExitCode AnalysisThread::Entry()
{
    int fileIndex{ 0 };
    for (std::shared_ptr<MediaFile> file : fileList)
    {
        int percentage = ((float)fileIndex / (float)fileList.size()) * 100.0;
        std::filesystem::path filePath{ file->FileName() };
        std::stringstream status;
        status << "Analyzing " << filePath.filename().string() 
               << " (" << fileIndex + 1 << " of " << fileList.size() << ", "
               << percentage << "%)";

        wxCommandEvent statusUpdateEvent
        {
             wxEVT_COMMAND_TEXT_UPDATED, StatusUpdateID 
        };
        statusUpdateEvent.SetInt(percentage);
        statusUpdateEvent.SetString(status.str());
        parent->GetEventHandler()->AddPendingEvent(statusUpdateEvent);

        file->Analyze(false);
        fileIndex++;
    }

    wxCommandEvent statusCompleteEvent
    {
            wxEVT_COMMAND_TEXT_UPDATED, StatusCompleteID 
    };
    parent->GetEventHandler()->AddPendingEvent(statusCompleteEvent);

    return 0;
}