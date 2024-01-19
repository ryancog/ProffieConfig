#include "mainmenu/dialogs/addconfig.h"

#include "core/appstate.h"
#include "core/defines.h"

#include "wx/filepicker.h"
#include "wx/string.h"
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

#include <fstream>

AddConfig::AddConfig(MainMenu* parent) : wxDialog(parent, wxID_ANY, "Add New Config", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), parent(parent) {
  createUI();
  bindEvents();

  FindWindowById(wxID_OK)->Disable();
}

void AddConfig::bindEvents() {
  Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        if (importExisting->GetValue()) {
          std::ifstream importConfig(chooseConfig->GetFileName().GetAbsolutePath().ToStdString());
          if (!importConfig.is_open()) {
            wxGenericMessageDialog(nullptr, "Failed to open config for import.", "Config Import Failure", wxCENTER | wxOK).ShowModal();
            return;
          }

          remove((CONFIG_DIR + configName->entry()->GetValue().ToStdString() + ".h").c_str());
          std::ofstream saveConfig(CONFIG_DIR + configName->entry()->GetValue().ToStdString() + ".h");
          if (!saveConfig.is_open()) {
            wxGenericMessageDialog(nullptr, "Failed to import config.", "Config Import Error", wxCENTER | 0x00000004).ShowModal();
            return;
          }

          saveConfig << importConfig.rdbuf();

          importConfig.close();
          saveConfig.close();
        } else {
          std::ofstream(CONFIG_DIR + configName->entry()->GetValue().ToStdString() + ".h").flush();
        }

        AppState::instance->addConfig(configName->entry()->GetValue().ToStdString());
        AppState::instance->saveState();
        parent->update();
        parent->configSelect->entry()->SetStringSelection(configName->entry()->GetValue());
        auto parentEvent = new wxCommandEvent(wxEVT_COMBOBOX, MainMenu::ID_ConfigSelect);
        wxQueueEvent(parent, parentEvent);

        event.Skip();
      }, wxID_OK);
  // We make sure to set itself to true that way it can't be deselected
  Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { importExisting->SetValue(true); createNew->SetValue(false); update(); }, ID_ImportExisting);
  Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { createNew->SetValue(true); importExisting->SetValue(false); update(); }, ID_CreateNew);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); });
  Bind(wxEVT_FILEPICKER_CHANGED, [&](wxCommandEvent&) {
    if (chooseConfig->GetFileName().FileExists()) {
      configName->entry()->SetValue(chooseConfig->GetFileName().GetName());
    }
    update();
  });
}

void AddConfig::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  sizer->SetMinSize(wxSize(400, -1));

  auto modeSelection = new wxBoxSizer(wxHORIZONTAL);
  createNew = new wxToggleButton(this, ID_CreateNew, "Create New Config");
  createNew->SetValue(true);
  importExisting = new wxToggleButton(this, ID_ImportExisting, "Import Existing Config");
  modeSelection->Add(createNew, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10));
  modeSelection->Add(importExisting, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));

  chooseConfigText = new wxStaticText(this, wxID_ANY, "Configuration to Import");
  chooseConfig = new wxFilePickerCtrl(this, ID_ChooseConfig, wxEmptyString, "Choose Configuration File to Import", "ProffieOS Configuration (*.h)|*.h");

  configNameText = new wxStaticText(this, wxID_ANY, "Configuration Name");
  configName = new pcTextCtrl(this, ID_ConfigName);

  invalidNameWarning = new wxStaticText(this, wxID_ANY, "Please enter a valid name");
  duplicateWarning = new wxStaticText(this, wxID_ANY, "Configuration with same name already exists");
  fileSelectionWarning = new wxStaticText(this, wxID_ANY, "Please choose a configuration file to import");

  sizer->Add(modeSelection, wxSizerFlags(0).Center());
  sizer->Add(chooseConfigText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
  sizer->Add(chooseConfig, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(configNameText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
  sizer->Add(configName, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(invalidNameWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->Add(duplicateWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->Add(fileSelectionWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->AddStretchSpacer(1);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags(0).Border(wxALL, 10).Expand());

  SetSizerAndFit(sizer);
  update();
}

void AddConfig::update() {
  auto duplicateConfigName = [&]() { for (const auto& config : AppState::instance->getConfigFileNames()) if (configName->entry()->GetValue() == config) return true; return false; }();
  auto configNameEmpty = configName->entry()->GetValue().empty();
  auto configNameInvalidCharacters = configName->entry()->GetValue().find_first_of(".\\,/!#$%^&*|?<>\"'") != std::string::npos;
  auto validConfigName = !configNameEmpty && !duplicateConfigName && !configNameInvalidCharacters;
  auto importingConfig = importExisting->GetValue();
  auto originFileSelected = chooseConfig->GetFileName().FileExists();

  duplicateWarning->Show(duplicateConfigName);
  invalidNameWarning->Show(!validConfigName);
  fileSelectionWarning->Show(importingConfig && !originFileSelected);

  FindWindowById(wxID_OK)->Enable(validConfigName && (importingConfig ? originFileSelected : true));

  chooseConfigText->Show(importingConfig);
  chooseConfig->Show(importingConfig);

  Layout(); // Although linux and windows seem to work without this, macOS requires it.
  Fit();
}
