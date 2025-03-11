#include "props.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/appstate.h"
#include "../../core/defines.h"

#include "wx/filepicker.h"
#include "wx/string.h"
#include <wx/event.h>
#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif
#include <wx/sizer.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

#include <fstream>

Props::Props(MainMenu* parent) : wxDialog(parent, wxID_ANY, "Add Prop File", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), parent(parent) {
  createUI();
  bindEvents();

  FindWindowById(wxID_OK)->Disable();
}

void Props::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        const auto propConfigPath{choosePropConfig->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propPath{chooseProp->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propName{choosePropConfig->GetFileName().GetName().ToStdString()};
        AppState::instance->addProp(propName, propPath, propConfigPath);
        AppState::instance->saveState();
        event.Skip();
    }, wxID_OK);
    Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_FILEPICKER_CHANGED, [&](wxCommandEvent&) { update(); });
}

void Props::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  sizer->SetMinSize(wxSize(400, -1));

  choosePropText = new wxStaticText(this, wxID_ANY, "ProffieOS Prop File");
  chooseProp = new wxFilePickerCtrl(this, ID_Prop, wxEmptyString, "Choose Prop File to Import", "C Header (*.h)|*.h");

  choosePropConfigText = new wxStaticText(this, wxID_ANY, "ProffieConfig Prop Config File");
  choosePropConfig = new wxFilePickerCtrl(this, ID_Prop, wxEmptyString, "Choose Prop Config File", "ProffieConfig Data File (*.pconf)|*.pconf");

  duplicateWarning = new wxStaticText(this, wxID_ANY, "Prop with same name already exists");
  fileSelectionWarning = new wxStaticText(this, wxID_ANY, "Please choose prop files to import");

  sizer->Add(choosePropText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
  sizer->Add(chooseProp, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(choosePropConfigText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
  sizer->Add(choosePropConfig, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  sizer->Add(duplicateWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->Add(fileSelectionWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
  sizer->AddStretchSpacer(1);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags(0).Border(wxALL, 10).Expand());

  SetSizerAndFit(sizer);
  update();
}

void Props::update() {
    auto duplicatePropFile = [&]() {
        for (const auto& propName : AppState::instance->getPropFileNames()) {
            if (choosePropConfig->GetFileName().GetName() == (propName + ".pconf")) return true;
        }
        return false;
    }();
    auto filesSelected{chooseProp->GetFileName().FileExists() and choosePropConfig->GetFileName().FileExists()};


    duplicateWarning->Show(duplicatePropFile);
    fileSelectionWarning->Show(not filesSelected);

    FindWindowById(wxID_OK)->Enable(not duplicatePropFile and filesSelected);

    Layout(); // Although linux and windows seem to work without this, macOS requires it.
    Fit();
}
