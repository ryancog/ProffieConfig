// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "mainmenu.h"

#include "core/defines.h"
#include "core/config/configuration.h"
#include "tools/arduino.h"
#include "tools/serialmonitor.h"

#include <wx/menu.h>

MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu() : wxFrame(nullptr, wxID_ANY, "ProffieConfig") {
  createUI();
  createMenuBar();
  createTooltips();
  bindEvents();

  Show(true);
}

void MainMenu::bindEvents() {
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::init(this); }, ID_ReRunSetup);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(ABOUT_MESSAGE, "About ProffieConfig", wxOK | wxICON_INFORMATION, this); }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION, this); }, ID_Copyright);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(activeEditor); }, ID_ExportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::importConfig(activeEditor); }, ID_ImportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/blob/master/docs"); }, ID_Docs);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/issues/new"); }, ID_Issue);
# if defined(__WXMSW__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { SerialMonitor::instance = new SerialMonitor; SerialMonitor::instance->Close(true); }, ID_OpenSerial);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise(); else SerialMonitor::instance = new SerialMonitor(this); }, ID_OpenSerial);
#endif


  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        if (boardSelect->GetValue() == "Select Device...") applyButton->Disable();
        else applyButton->Enable();
        if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Close(true);
      }, ID_DeviceSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, ID_RefreshDev);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(this, activeEditor); }, ID_ApplyChanges);

}

void MainMenu::createTooltips() {
  TIP(applyButton, "Apply the current configuration to the selected Proffieboard.");
  TIP(boardSelect, "Select the Proffieboard to connect to.\nThis will be an unrecognizable device identifier, but chances are there's only one which will show up.");
  TIP(refreshButton, "Refresh the detected boards.");
}

void MainMenu::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_ExportFile, "Export Config...", "Choose a location to save a copy of your config...");
  file->Append(ID_ImportFile, "Import Config...", "Choose a file to import...");
  file->AppendSeparator();
  file->AppendSeparator();
  file->Append(ID_ReRunSetup, "Re-Run First-Time Setup...", "Install Proffieboard Dependencies and View Tutorial");
  file->AppendSeparator();
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, "Copyright Notice");
  file->Append(wxID_EXIT);

  wxMenu* board = new wxMenu;
  board->Append(ID_OpenSerial, "Serial Monitor...\tCtrl+M", "Open a serial monitor to the proffieboard");

  wxMenu* tools = new wxMenu;
  tools->Append(ID_StyleEditor, "Style Editor...", "Open the ProffieOS style editor");

  wxMenu* help = new wxMenu;
  help->Append(ID_Docs, "Documentation...\tCtrl+H", "Open the ProffieConfig docs in your web browser");
  help->Append(ID_Issue, "Help/Bug Report...", "Open GitHub to submit issue");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  menuBar->Append(board, "&Board");
  menuBar->Append(tools, "&Tools");
  menuBar->Append(help, "&Help");
  SetMenuBar(menuBar);
}

void MainMenu::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  applyButton = new wxButton(this, ID_ApplyChanges, "Apply to Board");
  boardSelect = new wxComboBox(this, ID_DeviceSelect, "Select Board...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  refreshButton = new wxButton(this, ID_RefreshDev, "Refresh Connected Boards");

  SetSizerAndFit(sizer);
}
