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
#include "progress.h"

wxBoxSizer* MainWindow::master{nullptr};
wxButton* MainWindow::refreshButton{nullptr};
wxComboBox* MainWindow::windowSelect{nullptr};
wxComboBox* MainWindow::devSelect{nullptr};
wxButton* MainWindow::applyButton{nullptr};

GeneralPage* MainWindow::general{nullptr};
PropPage* MainWindow::prop{nullptr};
PresetsPage* MainWindow::presets{nullptr};
BladesPage* MainWindow::blades{nullptr};
HardwarePage* MainWindow::hardware{nullptr};

MainWindow* MainWindow::instance{nullptr};

MainWindow::MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  instance = this;
  CreateMenuBar();
  CreatePages();
  BindEvents();
  Configuration::readConfig(this);

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
          PropPage::update();
        } else if (windowSelect->GetValue() == "Presets") {
          general->Show(false);
          prop->Show(false);
          presets->Show(true);
          blades->Show(false);
          hardware->Show(false);
          PresetsPage::update();
        } else if (windowSelect->GetValue() == "Blades") {
          general->Show(false);
          prop->Show(false);
          presets->Show(false);
          blades->Show(true);
          hardware->Show(false);
          BladesPage::update();
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
      }, Misc::ID_DevSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, Misc::ID_RefreshDevButton);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(this); }, Misc::ID_ApplyButton);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxMessageBox("Tool for GUI Configuration and flashing of ProffieBoard (Created by Fredrik Hubbinette)\n\nTool Created by Ryryog25\n\nProffieOS v7.9 | Arduino Plugin v3.6.0 | Arduino CLI v0.34.2", "About ProffieConfig", wxOK | wxICON_INFORMATION);
      }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::outputConfig(); }, Misc::ID_GenFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(); }, Misc::ID_ExportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::importConfig(); }, Misc::ID_ImportFile);

  // Prop Page
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { PropPage::update(); UPDATEWINDOW; }, Misc::ID_PropSelect);
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) { PropPage::update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent&) { PropPage::update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_SPINCTRL, [&](wxCommandEvent) { PropPage::update(); UPDATEWINDOW; }, Misc::ID_PropOption);
  Bind(wxEVT_SPINCTRLDOUBLE, [&](wxCommandEvent&) { PropPage::update(); UPDATEWINDOW; }, Misc::ID_PropOption);

  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { Configuration::updateBladesConfig(); PresetsPage::update(); }, Misc::ID_BladeList);
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { Configuration::updateBladesConfig(); PresetsPage::update(); }, Misc::ID_PresetList);

  Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        // Update Style Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
          std::string style = PresetsPage::settings.presetsEditor->GetValue().ToStdString();
          style.erase(std::remove(style.begin(), style.end(), ' '), style.end());
          Configuration::presets[PresetsPage::settings.presetList->GetSelection()].styles[PresetsPage::settings.bladeList->GetSelection()].assign(style);
        } else PresetsPage::settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));

        Configuration::updateBladesConfig();
        PresetsPage::update();
      }, Misc::ID_PresetEditor);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        // Update Name Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
          std::string name = PresetsPage::settings.nameInput->GetValue().ToStdString();
          name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
          Configuration::presets[PresetsPage::settings.presetList->GetSelection()].name.assign(name);
        } else PresetsPage::settings.nameInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updateBladesConfig(); PresetsPage::update();
      }, Misc::ID_PresetName);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        // Update Dir Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
          std::string dir =  PresetsPage::settings.dirInput->GetValue().ToStdString();
          dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
          Configuration::presets[PresetsPage::settings.presetList->GetSelection()].dirs.assign(dir);
        } else PresetsPage::settings.dirInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updateBladesConfig(); PresetsPage::update();
      }, Misc::ID_PresetDir);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        // Update Track Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
          std::string track = PresetsPage::settings.trackInput->GetValue().ToStdString();
          track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
          Configuration::presets[PresetsPage::settings.presetList->GetSelection()].track.assign(track);
        } else PresetsPage::settings.trackInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updateBladesConfig(); PresetsPage::update();
      }, Misc::ID_PresetTrack);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Configuration::presets.push_back(Configuration::presetConfig());
        Configuration::presets[Configuration::presets.size() - 1].name = "New Preset";

        Configuration::updateBladesConfig();
        PresetsPage::update();
      }, Misc::ID_AddPreset);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (PresetsPage::settings.presetList->GetSelection() >= 0)
          Configuration::presets.erase(std::next(Configuration::presets.begin(), PresetsPage::settings.presetList->GetSelection()));

        Configuration::updateBladesConfig();
        PresetsPage::update();
      }, Misc::ID_RemovePreset);

  // Blades Page
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_BladeSelect);
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_SubBladeSelect);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;;
      }, Misc::ID_BladeType);
  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { Configuration::updateBladesConfig(); BladesPage::update(); }, Misc::ID_BladeOption);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_HASSELECTION) {
          Configuration::blades.insert(Configuration::blades.begin() + BladesPage::lastBladeSelection + 1, Configuration::Configuration::bladeConfig());
        } else {
          Configuration::blades.push_back(Configuration::Configuration::bladeConfig());
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_AddBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Configuration::blades[BladesPage::lastBladeSelection].isSubBlade = true;
        Configuration::blades[BladesPage::lastBladeSelection].subBlades.push_back(Configuration::Configuration::bladeConfig::subBladeInfo());
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_AddSubBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_HASSELECTION) {
          Configuration::blades.erase(Configuration::blades.begin() + BladesPage::lastBladeSelection);
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_RemoveBlade);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (BD_SUBHASSELECTION) {
          Configuration::blades[BladesPage::lastBladeSelection].subBlades.erase(Configuration::blades[BladesPage::lastBladeSelection].subBlades.begin() + BladesPage::lastSubBladeSelection);
          if (Configuration::blades[BladesPage::lastBladeSelection].subBlades.size() < 1) Configuration::blades[BladesPage::lastBladeSelection].isSubBlade = false;
          BladesPage::lastSubBladeSelection = -1;
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
      }, Misc::ID_RemoveSubBlade);
}

void MainWindow::CreateMenuBar() {
  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT);
  menuFile->Append(wxID_ABOUT);
  menuFile->Append(Misc::ID_Initialize, "Install Dependencies...\tCtrl+I", "Install Platform-Specific Proffieboard Dependencies");

  wxMenu *menuConfig = new wxMenu;
  menuConfig->Append(Misc::ID_GenFile, "Save Config\tCtrl+S", "Generate Config File");
  menuConfig->Append(Misc::ID_ExportFile, "Export Config...\t", "Choose a location to save a copy of your config...");
  menuConfig->Append(Misc::ID_ImportFile, "Import Config...\t", "Choose a file to import...");


  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuConfig, "&Config");
  SetMenuBar(menuBar);
}

void MainWindow::CreatePages() {
  master = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
  windowSelect = new wxComboBox(this, Misc::ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Presets", "Blades", "Hardware"}), wxCB_READONLY);
  refreshButton = new wxButton(this, Misc::ID_RefreshDevButton, "Refresh...", wxDefaultPosition, wxDefaultSize, 0);
  devSelect = new wxComboBox(this, Misc::ID_DevSelect, "Select Device...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  applyButton = new wxButton(this, Misc::ID_ApplyButton, "Apply to Board...", wxDefaultPosition, wxDefaultSize, 0);
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
