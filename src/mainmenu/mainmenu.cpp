// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "mainmenu.h"

#include "core/defines.h"
#include "core/utilities/misc.h"
#include "core/config/configuration.h"
#include "editor/editorwindow.h"
#include "onboard/onboard.h"
#include "tools/arduino.h"
#include "tools/serialmonitor.h"
#include "../resources/icons/icon-small.xpm"
#include "wx/event.h"

#include <wx/menu.h>

MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu() : wxFrame(nullptr, wxID_ANY, "ProffieConfig") {
  createUI();
  createMenuBar();
  createTooltips();
  bindEvents();

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  Show(true);
}

void MainMenu::bindEvents() {
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Destroy(); Onboard().run(); }, ID_ReRunSetup);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(ABOUT_MESSAGE, "About ProffieConfig", wxOK | wxICON_INFORMATION, this); }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION, this); }, ID_Copyright);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/blob/master/docs"); }, ID_Docs);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/issues/new"); }, ID_Issue);
# if defined(__WXMSW__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { SerialMonitor::instance = new SerialMonitor(this); SerialMonitor::instance->Close(true); }, ID_OpenSerial);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise(); else SerialMonitor::instance = new SerialMonitor(this); }, ID_OpenSerial);
#endif


  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) { update(); event.Skip(); });
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, ID_RefreshDev);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(this, activeEditor); }, ID_ApplyChanges);

  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        auto newEditor = new EditorWindow();
        if (!Configuration::readConfig("", newEditor)) {
          wxMessageBox("Error selecting configuration file!", "Config Error", wxOK | wxCENTER, this);
          delete newEditor;
          return;
        }
        activeEditor = newEditor;
        editors.push_back(newEditor);
      }, ID_ConfigSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { activeEditor->Show(); }, ID_EditConfig);

}

void MainMenu::createTooltips() {
  TIP(applyButton, "Apply the current configuration to the selected Proffieboard.");
  TIP(boardSelect, "Select the Proffieboard to connect to.\nThis will be an unrecognizable device identifier, but chances are there's only one which will show up.");
  TIP(refreshButton, "Refresh the detected boards.");
}

void MainMenu::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_ReRunSetup, "Re-Run First-Time Setup...", "Install Proffieboard Dependencies and View Tutorial");
  file->AppendSeparator();
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, "Copyright Notice");
  file->Append(wxID_EXIT);

  wxMenu* help = new wxMenu;
  help->Append(ID_Docs, "Documentation...\tCtrl+H", "Open the ProffieConfig docs in your web browser");
  help->Append(ID_Issue, "Help/Bug Report...", "Open GitHub to submit issue");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  menuBar->Append(help, "&Help");
  SetMenuBar(menuBar);
}

void MainMenu::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto headerSection = new wxBoxSizer(wxHORIZONTAL);
  auto titleSection = new wxBoxSizer(wxVERTICAL);
  auto title = new wxStaticText(this, wxID_ANY, "ProffieConfig");
  auto titleFont = title->GetFont();
  titleFont.MakeBold();
# if defined(__WXGTK__) || defined(__WXMSW__)
  titleFont.SetPointSize(20);
# elif defined (__WXOSX__)
  titleFont.SetPointSize(30);
#endif
  title->SetFont(titleFont);
  auto subTitle = new wxStaticText(this, wxID_ANY, "Created by Ryryog25");
  titleSection->Add(title, wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 10));
  titleSection->Add(subTitle, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10));
  headerSection->Add(titleSection, wxSizerFlags(0));
  headerSection->AddStretchSpacer(1);
  headerSection->Add(new wxStaticBitmap(this, wxID_ANY, icon_small_xpm), wxSizerFlags(0).Border(wxALL, 10));

  auto configSelectSection = new wxBoxSizer(wxHORIZONTAL);
  configSelect = new wxComboBox(this, ID_ConfigSelect, "Select Config...", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Select Config..."}), wxCB_READONLY);
  addConfig = new wxButton(this, ID_AddConfig, "Add", wxDefaultPosition, wxSize(50, -1), wxBU_EXACTFIT);
  removeConfig = new wxButton(this, ID_RemoveConfig, "Remove", wxDefaultPosition, wxSize(75, -1), wxBU_EXACTFIT);
  removeConfig->Disable();
  configSelectSection->Add(configSelect, wxSizerFlags(1).Border(wxALL, 5).Expand());
  configSelectSection->Add(addConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());
  configSelectSection->Add(removeConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());

  auto boardControls = new wxBoxSizer(wxHORIZONTAL);
  boardSelect = new wxComboBox(this, ID_DeviceSelect, "Select Board...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  refreshButton = new wxButton(this, ID_RefreshDev, "Refresh Boards");
  boardControls->Add(refreshButton, wxSizerFlags(0).Border(wxALL, 5));
  boardControls->Add(boardSelect, wxSizerFlags(1).Border(wxALL, 5));

  auto options = new wxBoxSizer(wxVERTICAL);
  applyButton = new wxButton(this, ID_ApplyChanges, "Apply Selected Configuration to Board");
  applyButton->Disable();
  editConfig = new wxButton(this, ID_EditConfig, "Edit Selected Configuration");
  editConfig->Disable();
  openSerial = new wxButton(this, ID_OpenSerial, "Open Serial Monitor");
  openSerial->Disable();
  options->Add(applyButton, wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(editConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(openSerial, wxSizerFlags(0).Border(wxALL, 5).Expand());

  sizer->Add(headerSection, wxSizerFlags(0).Expand());
  sizer->AddSpacer(20);
  sizer->Add(configSelectSection, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(boardControls, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(options, wxSizerFlags(0).Border(wxALL, 5).Expand());
  SetSizerAndFit(sizer);
}

void MainMenu::update() {
  auto configSelected = configSelect->GetValue() != "Select Config...";
  auto boardSelected = boardSelect->GetValue() != "Select Board...";
  auto recoverySelected = boardSelect->GetValue() == "BOOTLOADER RECOVERY";

  applyButton->Enable(configSelected && boardSelected);
  editConfig->Enable(configSelected);
  removeConfig->Enable(configSelected);
  openSerial->Enable(boardSelected && !recoverySelected);

  for (auto editor = editors.begin(); editor < editors.end();) {
    if (!(*editor)->IsShown()) {
      if (&**editor == &*activeEditor) activeEditor = nullptr;
      (*editor)->Destroy();
      editor = editors.erase(editor);
      continue;
    }
    editor++;
  }
}
