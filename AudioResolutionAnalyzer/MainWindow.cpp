// MainWindow.cpp - Defines the MainWindow class.
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

#include "MainWindow.h"

enum ID
{
    File = 1,
    Analyze
};

enum Column
{
    FileName = 0,
    BitDepth,
    SampleRate,
    IsUpscaled
};

// catch the event from the thread
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_COMMAND(AnalysisThread::StatusUpdateID, wxEVT_COMMAND_TEXT_UPDATED,
            MainWindow::OnStatusUpdate)
EVT_COMMAND(AnalysisThread::StatusCompleteID, wxEVT_COMMAND_TEXT_UPDATED,
            MainWindow::OnAnalysisComplete)
END_EVENT_TABLE()

MainWindow::MainWindow(wxString programInfo) : 
    wxFrame(nullptr, wxID_ANY, programInfo)
{
    this->programInfo = programInfo;

    wxPanel* topPanel = new wxPanel{ this, wxID_ANY, wxDefaultPosition };
    wxPanel* bottomPanel = new wxPanel{ this, wxID_ANY, wxDefaultPosition };

    wxBoxSizer* frameSizer = new wxBoxSizer{ wxVERTICAL };
    wxBoxSizer* bottomSizer = new wxBoxSizer{ wxHORIZONTAL };

    wxMenu *fileMenu = new wxMenu;
    openMenuItem = new wxMenuItem
    {
        fileMenu, ID::File, "&Open...\tCtrl-O", 
        "Opens audio files for analysis" 
    };
    fileMenu->Append(openMenuItem);
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT);
 
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
 
    SetMenuBar( menuBar );
 
    CreateStatusBar();
    SetStatusText("Ready");

    fileListView = new wxListView{ topPanel, wxID_ANY, 
                                   wxDefaultPosition, wxSize{600, 400} };
    fileListView->AppendColumn("File Name", wxLIST_FORMAT_LEFT, 300);
    fileListView->AppendColumn("Bit Depth");
    fileListView->AppendColumn("Sample Rate");
    fileListView->AppendColumn("Is Upscaled");

    analyzeButton = new wxButton{ bottomPanel, ID::Analyze, "Analyze" };
    progressBar = new wxGauge{ bottomPanel, wxID_ANY, 100, 
                               wxDefaultPosition, wxSize{ 200, 20 } };
    bottomSizer->Add(analyzeButton, 0, wxALL, 5);
    bottomSizer->Add(progressBar, 0, wxALL | wxCENTER, 5);
    bottomPanel->SetSizerAndFit(bottomSizer);
 
    frameSizer->Add(topPanel, 1, wxEXPAND);
    frameSizer->Add(bottomPanel, 0, wxEXPAND);
    SetSizerAndFit(frameSizer);

    logger = std::make_shared<Logging::Logger>();
    logFile = std::make_shared<Logging::LogFile>();
    logger->Add(logFile.get());

    Bind(wxEVT_MENU, &MainWindow::OnOpen, this, ID::File);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_BUTTON, &MainWindow::OnAnalyze, this, ID::Analyze);
}

void MainWindow::OnOpen(wxCommandEvent& event)
{
    wxFileDialog dialog(this, _("Open media file"), "", "",
                        "Media files (*.wav;*.flac)|*.wav;*.flac", 
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
        fileList.clear();
        wxArrayString paths;
        dialog.GetPaths(paths);

        for (wxString path : paths)
        {
            std::string pathString = path.ToStdString();
            std::filesystem::path filePath{ pathString };
            std::string fileName{ filePath.stem().string() };
            std::string extension { filePath.extension().string() };

            std::shared_ptr<MediaFile> file;
            if (extension == ".wav")
            {
                file = std::make_shared<WaveFile>(pathString, logger);
            }  
            else if (extension == ".flac")
            {
                file = std::make_shared<FlacFile>(pathString, logger);
            }   
            else
            {
                wxMessageBox("Unsupported file type!", "Error", 
                            wxOK | wxICON_ERROR);
            }

            file->Open();
            if (!file->IsOpen())
                ShowError("Unable to open file!");

            fileList.push_back(file);
        }
            
        PopulateFileListView();
    }
}

void MainWindow::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
    std::stringstream about;
    about << programInfo << std::endl << PROGRAM_COPYRIGHT;
    wxMessageBox(about.str(), "About", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnAnalyze(wxCommandEvent& event)
{
    analyzeButton->Enable(false);
    openMenuItem->Enable(false);

    AnalysisThread* thread = new AnalysisThread{ this, fileList };
    wxThreadError error = thread->Create();
    if (error != wxTHREAD_NO_ERROR)
        ShowError("Could not create thread to analyze audio!");

    error = thread->Run();
    if (error != wxTHREAD_NO_ERROR)
        ShowError("Could not run thread to analyze audio!");
}

void MainWindow::OnStatusUpdate(wxCommandEvent& event)
{
    progressBar->SetValue(event.GetInt());
    SetStatusText(event.GetString());
}

void MainWindow::OnAnalysisComplete(wxCommandEvent& event)
{
    analyzeButton->Enable(true);
    openMenuItem->Enable(true);
    progressBar->SetValue(100);
    UpdateFileListView();
    SetStatusText("Ready");
}

void MainWindow::PopulateFileListView()
{
    fileListView->DeleteAllItems();

    int itemIndex{ 0 };
    for (std::shared_ptr<MediaFile> file : fileList)
    {
        std::filesystem::path path{ file->FileName() };
        fileListView->InsertItem(itemIndex, path.filename().string());
        itemIndex++;
    }
}

void MainWindow::UpdateFileListView()
{
    int itemIndex{ 0 };
    for (std::shared_ptr<MediaFile> file : fileList)
    {
        std::filesystem::path path{ file->FileName() };
        fileListView->SetItem(itemIndex, Column::FileName, 
                              path.filename().string());
        fileListView->SetItem(itemIndex, Column::BitDepth, 
                              std::to_string(file->BitsPerSample()));
        fileListView->SetItem(itemIndex, Column::SampleRate, 
                              std::to_string(file->SampleRate()));
        if (file->IsUpscaled())
            fileListView->SetItem(itemIndex, Column::IsUpscaled, "Yes");
        else
            fileListView->SetItem(itemIndex, Column::IsUpscaled, "No");
        itemIndex++;
    }
}

void MainWindow::ShowError(wxString message)
{
    wxMessageBox(message, "AudioResolutionAnalyzer", wxOK | wxICON_ERROR);
}