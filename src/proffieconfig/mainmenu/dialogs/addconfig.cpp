#include "addconfig.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/filepicker.h>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/tglbtn.h>
#include <wx/button.h>
#include <wx/sysopt.h>

AddConfig::AddConfig(MainMenu *parent) : 
    wxDialog(parent, wxID_ANY, "Add New Config", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) {
#   ifdef __WXOSX__
    wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, true);
#   endif

    createUI();
    bindEvents();

    FindWindowById(wxID_OK)->Disable();
}

void AddConfig::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (importExisting->GetValue()) {
            existingPath = chooseConfig->GetFileName().GetAbsolutePath().ToStdString();
        } 

        configName = configNameEntry->entry()->GetValue().ToStdString();
        EndModal(wxID_OK);
    }, wxID_OK);
    // We make sure to set itself to true that way it can't be deselected
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { importExisting->SetValue(true); createNew->SetValue(false); update(); }, ID_ImportExisting);
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { createNew->SetValue(true); importExisting->SetValue(false); update(); }, ID_CreateNew);
    Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_FILEPICKER_CHANGED, [&](wxCommandEvent&) {
        if (chooseConfig->GetFileName().FileExists()) {
            configNameEntry->entry()->SetValue(chooseConfig->GetFileName().GetName());
        }
        update();
    });
}

void AddConfig::createUI() {
  auto *sizer{new wxBoxSizer(wxVERTICAL)};
  sizer->SetMinSize(wxSize(400, -1));

  auto *modeSelection{new wxBoxSizer(wxHORIZONTAL)};
  createNew = new wxToggleButton(this, ID_CreateNew, "Create New Config");
  createNew->SetValue(true);
  importExisting = new wxToggleButton(this, ID_ImportExisting, "Import Existing Config");
  modeSelection->Add(createNew, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10));
  modeSelection->Add(importExisting, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));

  mChooseConfigText = new wxStaticText(this, wxID_ANY, "Configuration to Import");
  chooseConfig = new wxFilePickerCtrl(this, ID_ChooseConfig, wxEmptyString, "Choose Configuration File to Import", "ProffieOS Configuration (*.h)|*.h");

  configNameEntry = new PCUI::Text(this, ID_ConfigName, {}, 0, "Configuration Name");

  mInvalidNameWarning = new wxStaticText(this, wxID_ANY, "Please enter a valid name");
  mDuplicateWarning = new wxStaticText(this, wxID_ANY, "Configuration with same name already exists");
  mFileSelectionWarning = new wxStaticText(this, wxID_ANY, "Please choose a configuration file to import");

  sizer->Add(modeSelection, wxSizerFlags(0).Center());
  sizer->Add(mChooseConfigText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
  sizer->Add(chooseConfig, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(configNameEntry, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(mInvalidNameWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->Add(mDuplicateWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->Add(mFileSelectionWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->AddStretchSpacer(1);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags(0).Border(wxALL, 10).Expand());

  SetSizerAndFit(sizer);
  update();
}

void AddConfig::update() {
  auto duplicateConfigName = [&]() { 
      // for (const auto& config : getConfigFileNames()) {
      //     if (configName->entry()->GetValue() == config) return true;
      // }
      return false;
  }();

  auto configNameEmpty = configNameEntry->entry()->GetValue().empty();
  auto configNameInvalidCharacters = configNameEntry->entry()->GetValue().find_first_of(".\\,/!#$%^&*|?<>\"'") != string::npos;
  auto validConfigName = !configNameEmpty && !duplicateConfigName && !configNameInvalidCharacters;
  auto importingConfig = importExisting->GetValue();
  auto originFileSelected = chooseConfig->GetFileName().FileExists();

  mDuplicateWarning->Show(duplicateConfigName);
  mInvalidNameWarning->Show(!validConfigName);
  mFileSelectionWarning->Show(importingConfig && !originFileSelected);

  FindWindowById(wxID_OK)->Enable(validConfigName && (importingConfig ? originFileSelected : true));

  mChooseConfigText->Show(importingConfig);
  chooseConfig->Show(importingConfig);

  Layout(); // Although linux and windows seem to work without this, macOS requires it.
  Fit();
}
