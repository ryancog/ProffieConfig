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

MainWindow* MainWindow::instance;
MainWindow::MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  instance = this;
  config = new Configuration();
  CreateMenuBar();
  CreatePages();
  BindEvents();
  SetSizerAndFit(master);
}

void MainWindow::BindEvents() {
  // Main Window
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent(progDialog, (Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        // TODO general->update();
        if (windowSelect->GetValue() == "General") {
          general->Show(true);
          prop->Show(false);
          presets->Show(false);
          blades->Show(false);
          hardware->Show(false);
        } else if (windowSelect->GetValue() == "Prop File") {
          general->Show(false);
          prop->Show(true);
          presets->Show(false);
          blades->Show(false);
          hardware->Show(false);
          PropPage::instance->update();
        } else if (windowSelect->GetValue() == "Presets") {
          general->Show(false);
          prop->Show(false);
          presets->Show(true);
          blades->Show(false);
          hardware->Show(false);
          PresetsPage::instance->update();
        } else if (windowSelect->GetValue() == "Blades") {
          general->Show(false);
          prop->Show(false);
          presets->Show(false);
          blades->Show(true);
          hardware->Show(false);
          BladesPage::instance->update();
        } else if (windowSelect->GetValue() == "Hardware") {
          general->Show(false);
          prop->Show(false);
          presets->Show(false);
          blades->Show(false);
          hardware->Show(true);
        }
        UPDATEWINDOW;
      }, Misc::ID_WindowSelect);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::init(); }, Misc::ID_Initialize);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        if (devSelect->GetValue() == "Select Device...") applyButton->Disable();
        else applyButton->Enable();
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
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_HASSELECTION) {
          Configuration::instance->blades.insert(Configuration::instance->blades.begin() + BladesPage::instance->lastBladeSelection + 1, Configuration::Configuration::bladeConfig());
        } else {
          Configuration::instance->blades.push_back(Configuration::Configuration::bladeConfig());
        }
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_AddBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].isSubBlade = true;
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.push_back(Configuration::bladeConfig::subBladeInfo());
        if (Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.size() <= 1) Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.push_back(Configuration::bladeConfig::subBladeInfo());
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_AddSubBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_HASSELECTION) {
          Configuration::instance->blades.erase(Configuration::instance->blades.begin() + BladesPage::instance->lastBladeSelection);
        }
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_RemoveBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_SUBHASSELECTION) {
          Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.erase(Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.begin() + BladesPage::instance->lastSubBladeSelection);
          if (Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.size() <= 1) {
            Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.clear();
            Configuration::instance->blades[BladesPage::instance->lastBladeSelection].isSubBlade = false;
          }
          BladesPage::instance->lastSubBladeSelection = -1;
        }
        Configuration::instance->updateBladesConfig();
        BladesPage::instance->update();
        UPDATEWINDOW;
      }, Misc::ID_RemoveSubBlade);
}

void MainWindow::CreateMenuBar() {
  wxMenu *menuFile = new wxMenu;

  menuFile->Append(Misc::ID_GenFile, "Save Config\tCtrl+S", "Generate Config File");
  menuFile->Append(Misc::ID_VerifyConfig, "Verify Config...\tCtrl+R", "Generate Config and Compile to test...");

  menuFile->Append(Misc::ID_ExportFile, "Export Config...\t", "Choose a location to save a copy of your config...");
  menuFile->Append(Misc::ID_ImportFile, "Import Config...\t", "Choose a file to import...");

  menuFile->Append(Misc::ID_Initialize, "Install Dependencies...\t", "Install Platform-Specific Proffieboard Dependencies");

  menuFile->Append(wxID_ABOUT);
  menuFile->Append(wxID_EXIT);


  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
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

  general = new GeneralPage(this);
  prop = new PropPage(this);
  presets = new PresetsPage(this);
  blades = new BladesPage(this);
  hardware = new HardwarePage(this);

  prop->Show(false);
  presets->Show(false);
  blades->Show(false);
  hardware->Show(false);

  master->Add(options, wxSizerFlags(0).Expand());
  master->Add(general, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(prop, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(presets, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(blades, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(hardware, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(master); // use the sizer for layout and set size and hints
}
