// MainWindow.h - Declares the MainWindow class.
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
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <filesystem>
#include <memory>
#include <vector>
#include <sstream>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "MediaFile.h"
#include "WaveFile.h"
#include "FlacFile.h"
#include "Logging.h"
#include "AnalysisThread.h"
#include "Version.h"

class MainWindow : public wxFrame
{
public:
    MainWindow(wxString programInfo);
private:
    wxMenuItem* openMenuItem;
    wxListView* fileListView;
    wxButton* analyzeButton;
    wxGauge* progressBar;
    wxString programInfo;
    std::vector<std::shared_ptr<MediaFile>> fileList;
    std::shared_ptr<Logging::Logger> logger; 

    void OnOpen(wxCommandEvent& event);

    void OnExit(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    void OnAnalyze(wxCommandEvent& event);

    void OnStatusUpdate(wxCommandEvent& event);

    void OnAnalysisComplete(wxCommandEvent& event);

    void PopulateFileListView();

    void UpdateFileListView();

    void ShowError(wxString message);

    DECLARE_EVENT_TABLE()
};

#endif