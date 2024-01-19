// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "mainmenu.h"

#include "core/defines.h"
#include "core/appstate.h"
#include "core/utilities/misc.h"
#include "core/config/configuration.h"
#include "editor/editorwindow.h"
#include "onboard/onboard.h"
#include "mainmenu/dialogs/addconfig.h"
#include "tools/arduino.h"
#include "tools/serialmonitor.h"
#include "../resources/icons/icon-small.xpm"

#include "ui/pccombobox.h"

#include <wx/event.h>
#include <wx/menu.h>
#include <wx/aboutdlg.h>
#include <wx/generic/aboutdlgg.h>
#include <wx/msgdlg.h>
#include <wx/statbmp.h>

MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu(wxWindow* parent) : wxFrame(parent, wxID_ANY, "ProffieConfig") {
  createUI();
  createMenuBar();
  createTooltips();
  bindEvents();
  update();

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  CenterOnScreen();
  Show(true);
}

void MainMenu::bindEvents() {
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    AppState::instance->saveState();
    for (const auto& editor : editors) {
      if (editor->IsShown() && event.CanVeto()) {
        if (wxMessageDialog(this, "There are editors open, are you sure you want to exit?\n\nAny unsaved changes will be lost!", "Open Editor(s)", wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION).ShowModal() == wxID_NO) {
          event.Veto();
          return;
        } else
          break;
      }
    }
    event.Skip();
  });
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
      wxMessageDialog(this, ((Misc ::MessageBoxEvent *)&event)->message, ((Misc ::MessageBoxEvent *)&event)->caption, ((Misc ::MessageBoxEvent *)&event)->style).ShowModal();
    }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(); Onboard::instance = new Onboard(); }, ID_ReRunSetup);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxAboutDialogInfo aboutInfo;
        aboutInfo.SetDescription(
            "All-in-one Proffieboard Management Utility\n"
            "\n"
            "ProffieOS v" PROFFIEOS_VERSION " | Arduino CLI v" ARDUINO_CLI_VERSION
            );
        aboutInfo.SetVersion(VERSION);
        aboutInfo.SetWebSite("https://github.com/Ryryog25/ProffieConfig");
        aboutInfo.SetCopyright("Copyright (C) 2024 Ryan Ogurek");
        aboutInfo.SetName("ProffieConfig");
        wxAboutBox(aboutInfo, this);
      }, wxID_ABOUT);

  Bind(
    wxEVT_MENU,
    [&](wxCommandEvent &) {
      wxMessageDialog(this, COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION).ShowModal();
    },
    ID_Copyright);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/blob/master/docs"); }, ID_Docs);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/issues/new"); }, ID_Issue);

  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) { update(); event.Skip(); });
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, ID_RefreshDev);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(this, activeEditor); }, ID_ApplyChanges);
# if defined(__WXMSW__)
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { SerialMonitor::instance = new SerialMonitor(this); SerialMonitor::instance->Close(true); }, ID_OpenSerial);
# else
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise(); else SerialMonitor::instance = new SerialMonitor(this); }, ID_OpenSerial);
#endif
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        if (configSelect->entry()->GetValue() == "Select Config...") return;

        for (auto editor : editors) {
          if (configSelect->entry()->GetValue() == editor->getOpenConfig()) {
            activeEditor = editor;
            update();
            return;
          }
        }

        auto newEditor = new EditorWindow(configSelect->entry()->GetValue().ToStdString(), this);
        if (!Configuration::readConfig(CONFIG_DIR + configSelect->entry()->GetValue().ToStdString() + ".h", newEditor)) {
          wxMessageDialog(this, "Error reading configuration file!", "Config Error", wxOK | wxCENTER).ShowModal();
          newEditor->Destroy();
          AppState::instance->removeConfig(configSelect->entry()->GetValue().ToStdString());
          update();
          return;
        }
        activeEditor = newEditor;
        editors.push_back(newEditor);

        update();
      }, ID_ConfigSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { activeEditor->Show(); activeEditor->Raise(); }, ID_EditConfig);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { AddConfig(this).ShowModal(); }, ID_AddConfig);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent &) {
      if (wxMessageDialog(this, "Are you sure you want to deleted the selected configuration?\n\nThis action cannot be undone!", "Delete Config", wxYES_NO | wxNO_DEFAULT | wxCENTER).ShowModal() == wxID_YES) {
          activeEditor->Close(true);
          remove((CONFIG_DIR + configSelect->entry()->GetValue().ToStdString() + ".h").c_str());
          AppState::instance->removeConfig(configSelect->entry()->GetValue().ToStdString());
          AppState::instance->saveState();
          activeEditor = nullptr;
          update();
        }
    },
    ID_RemoveConfig);
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
  auto subTitle = new wxStaticText(this, wxID_ANY, "Created by Ryryog25\n\n");
  titleSection->Add(title, wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 10));
  titleSection->Add(subTitle, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10));
  headerSection->Add(titleSection, wxSizerFlags(0));
  headerSection->AddStretchSpacer(1);
  headerSection->Add(new wxStaticBitmap(this, wxID_ANY, wxIcon(icon_small_xpm)), wxSizerFlags(0).Border(wxALL, 10));

  auto configSelectSection = new wxBoxSizer(wxHORIZONTAL);
  configSelect = new pcComboBox(this, ID_ConfigSelect, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Select Config..."}), wxCB_READONLY);
  addConfig = new wxButton(this, ID_AddConfig, "Add", wxDefaultPosition, wxSize(50, -1), wxBU_EXACTFIT);
  removeConfig = new wxButton(this, ID_RemoveConfig, "Remove", wxDefaultPosition, wxSize(75, -1), wxBU_EXACTFIT);
  removeConfig->Disable();
  configSelectSection->Add(configSelect, wxSizerFlags(1).Border(wxALL, 5).Expand());
  configSelectSection->Add(addConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());
  configSelectSection->Add(removeConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());

  auto boardControls = new wxBoxSizer(wxHORIZONTAL);
# ifdef __WXMSW__
  auto boardEntries = Misc::createEntries({"Select Board...", "BOOTLOADER RECOVERY"});
# else
  auto boardEntries = Misc::createEntries({"Select Board..."});
# endif
  boardSelect = new pcComboBox(this, ID_DeviceSelect, "", wxDefaultPosition, wxDefaultSize, boardEntries, wxCB_READONLY);
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
  sizer->Add(configSelectSection, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(boardControls, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(options, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->AddSpacer(20); // There's a sizing issue I need to figure out... for now we give it a chin

  SetSizerAndFit(sizer);
}

void MainMenu::update() {
  auto lastConfig = configSelect->entry()->GetValue();
  configSelect->entry()->Clear();
  configSelect->entry()->Append("Select Config...");
  for (const auto& config : AppState::instance->getConfigFileNames()) {
    configSelect->entry()->Append(config);
  }
  configSelect->entry()->SetValue(lastConfig);
  if (configSelect->entry()->GetSelection() == -1) configSelect->entry()->SetSelection(0);

  for (auto editor = editors.begin(); editor < editors.end();) {
    if (!(*editor)->IsShown()) {
      if (activeEditor != nullptr && &**editor == &*activeEditor) {
        editor++;
        continue;
      }
      (*editor)->Destroy();
      editor = editors.erase(editor);
      continue;
    }
    editor++;
  }

  auto configSelected = configSelect->entry()->GetValue() != "Select Config...";
  auto boardSelected = boardSelect->entry()->GetValue() != "Select Board...";
  auto recoverySelected = boardSelect->entry()->GetValue().find("BOOTLOADER") != std::string::npos;

  applyButton->Enable(configSelected && boardSelected);
  editConfig->Enable(configSelected);
  removeConfig->Enable(configSelected);
  openSerial->Enable(boardSelected && !recoverySelected);
}
