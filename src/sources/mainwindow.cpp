#include "mainwindow.h"

#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>

#include "configuration.h"
#include "defines.h"
#include "arduino.h"
#include "misc.h"
#include "serialmonitor.h"

#include <bladespage.h>
#include <generalpage.h>
#include <hardwarepage.h>
#include <presetspage.h>
#include <proppage.h>

MainWindow* MainWindow::instance;
MainWindow::MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  instance = this;
  Configuration::instance = new Configuration();
  CreateMenuBar();
  CreatePages();
  BindEvents();
  SetSizerAndFit(master);
  Show(true);
}

void MainWindow::BindEvents() {
  // Main Window
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent(progDialog, (Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { if (SerialMonitor::instance == nullptr) SerialMonitor::instance = new SerialMonitor(); else SerialMonitor::instance->Raise(); }, Misc::ID_OpenSerial);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        // TODO GeneralPage::instance->update();
        if (windowSelect->GetValue() == "General") {
          GeneralPage::instance->Show(true);
          PropPage::instance->Show(false);
          PresetsPage::instance->Show(false);
          BladesPage::instance->Show(false);
          HardwarePage::instance->Show(false);
        } else if (windowSelect->GetValue() == "Prop File") {
          GeneralPage::instance->Show(false);
          PropPage::instance->Show(true);
          PresetsPage::instance->Show(false);
          BladesPage::instance->Show(false);
          HardwarePage::instance->Show(false);
          PropPage::instance->update();
        } else if (windowSelect->GetValue() == "Presets") {
          GeneralPage::instance->Show(false);
          PropPage::instance->Show(false);
          PresetsPage::instance->Show(true);
          BladesPage::instance->Show(false);
          HardwarePage::instance->Show(false);
          PresetsPage::instance->update();
        } else if (windowSelect->GetValue() == "Blades") {
          GeneralPage::instance->Show(false);
          PropPage::instance->Show(false);
          PresetsPage::instance->Show(false);
          BladesPage::instance->Show(true);
          HardwarePage::instance->Show(false);
          BladesPage::instance->update();
        } else if (windowSelect->GetValue() == "Hardware") {
          GeneralPage::instance->Show(false);
          PropPage::instance->Show(false);
          PresetsPage::instance->Show(false);
          BladesPage::instance->Show(false);
          HardwarePage::instance->Show(true);
        }
        UPDATEWINDOW;
      }, Misc::ID_WindowSelect);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::init(); }, Misc::ID_Initialize);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        if (devSelect->GetValue() == "Select Device...") applyButton->Disable();
        else applyButton->Enable();
        if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Close(true);
      }, Misc::ID_DeviceSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(); }, Misc::ID_RefreshDev);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(); }, Misc::ID_ApplyChanges);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxMessageBox("Tool for GUI Configuration and flashing of ProffieBoard (Created by Fredrik Hubbinette)\n\nTool Created by Ryryog25\nhttps://github.com/ryryog25/ProffieConfig\n\nProffieOS v7.9 | Arduino Plugin v3.6.0 | Arduino CLI v0.34.2", "About ProffieConfig", wxOK | wxICON_INFORMATION);
      }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->outputConfig(); }, Misc::ID_GenFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(); }, Misc::ID_VerifyConfig);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->exportConfig(); }, Misc::ID_ExportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->importConfig(); }, Misc::ID_ImportFile);

  // Prop Page
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { PropPage::instance->update(); UPDATEWINDOW; }, Misc::ID_PropSelect);
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) { PropPage::instance->update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent&) { PropPage::instance->update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_SPINCTRL, [&](wxCommandEvent) { PropPage::instance->update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_SPINCTRLDOUBLE, [&](wxCommandEvent&) { PropPage::instance->update(); UPDATEWINDOW; }, Misc::ID_PropOption);

  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { Configuration::instance->updateBladesConfig(); PresetsPage::instance->update(); }, Misc::ID_BladeList);
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { Configuration::instance->updateBladesConfig(); PresetsPage::instance->update(); }, Misc::ID_PresetList);

  Bind(wxEVT_TEXT, [&](wxCommandEvent&) { PresetsPage::instance->updatePresetEditor(); }, Misc::ID_PresetEditor);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) { PresetsPage::instance->updatePresetName(); }, Misc::ID_PresetName);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) { PresetsPage::instance->updatePresetDir(); }, Misc::ID_PresetDir);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) { PresetsPage::instance->updatePresetTrack(); }, Misc::ID_PresetTrack);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Configuration::instance->presets.push_back(Configuration::presetConfig());
        Configuration::instance->presets[Configuration::instance->presets.size() - 1].name = "NewPreset";

        Configuration::instance->updateBladesConfig();
        PresetsPage::instance->update();
      }, Misc::ID_AddPreset);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (PresetsPage::instance->settings.presetList->GetSelection() >= 0) {
          Configuration::instance->presets.erase(std::next(Configuration::instance->presets.begin(), PresetsPage::instance->settings.presetList->GetSelection()));

          Configuration::instance->updateBladesConfig();
          PresetsPage::instance->update();
        }
      }, Misc::ID_RemovePreset);

  // Blades Page
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_BladeSelect);
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_SubBladeSelect);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;;
      }, Misc::ID_BladeType);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { Configuration::instance->updateBladesConfig(); BladesPage::instance->update(); }, Misc::ID_BladeOption);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { BladesPage::instance->addBlade(); UPDATEWINDOW; }, Misc::ID_AddBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { BladesPage::instance->addSubBlade(); UPDATEWINDOW; }, Misc::ID_AddSubBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { BladesPage::instance->removeBlade(); UPDATEWINDOW; }, Misc::ID_RemoveBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { BladesPage::instance->removeSubBlade(); UPDATEWINDOW; }, Misc::ID_RemoveSubBlade);
}

void MainWindow::CreateMenuBar() {
  wxMenu *file = new wxMenu;

  file->Append(Misc::ID_GenFile, "Save Config\tCtrl+S", "Generate Config File");
  file->Append(Misc::ID_VerifyConfig, "Verify Config...\tCtrl+R", "Generate Config and Compile to test...");

  file->Append(Misc::ID_ExportFile, "Export Config...\t", "Choose a location to save a copy of your config...");
  file->Append(Misc::ID_ImportFile, "Import Config...\t", "Choose a file to import...");

  file->Append(Misc::ID_Initialize, "Install Dependencies...\t", "Install Platform-Specific Proffieboard Dependencies");

  file->Append(wxID_ABOUT);
  file->Append(wxID_EXIT);

  wxMenu* board = new wxMenu;

  board->Append(Misc::ID_OpenSerial, "Serial Monitor...\tCtrl+M", "Open a serial monitor to the proffieboard");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  //menuBar->Append(board, "&Board");
  SetMenuBar(menuBar);
}

void MainWindow::CreatePages() {
  master = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
  windowSelect = new wxComboBox(this, Misc::ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blades", "Presets"  /*, "Hardware"*/}), wxCB_READONLY);
  refreshButton = new wxButton(this, Misc::ID_RefreshDev, "Refresh...", wxDefaultPosition, wxDefaultSize, 0);
  devSelect = new wxComboBox(this, Misc::ID_DeviceSelect, "Select Device...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  applyButton = new wxButton(this, Misc::ID_ApplyChanges, "Apply to Board...", wxDefaultPosition, wxDefaultSize, 0);
  applyButton->Disable();
  options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->AddStretchSpacer(1);
  options->Add(refreshButton, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(devSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(applyButton, wxSizerFlags(0).Border(wxALL, 10));

  GeneralPage::instance = new GeneralPage(this);
  PropPage::instance = new PropPage(this);
  PresetsPage::instance = new PresetsPage(this);
  BladesPage::instance = new BladesPage(this);
  HardwarePage::instance = new HardwarePage(this);

  PropPage::instance->Show(false);
  BladesPage::instance->Show(false);
  PresetsPage::instance->Show(false);
  HardwarePage::instance->Show(false);

  master->Add(options, wxSizerFlags(0).Expand());
  master->Add(GeneralPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(PropPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(PresetsPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(BladesPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(HardwarePage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(master); // use the sizer for layout and set size and hints
}
