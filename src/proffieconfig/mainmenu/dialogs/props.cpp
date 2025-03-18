#include "props.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/appstate.h"

#include <wx/event.h>
#include <wx/filepicker.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

Props::Props(MainMenu* parent) : wxDialog(parent, wxID_ANY, "Prop Files", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), parent(parent) {
  createUI();
  bindEvents();
  update();

  FindWindowById(wxID_OK)->Disable();
}

void Props::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        const auto propConfigPath{choosePropConfig->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propPath{chooseProp->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propName{choosePropConfig->GetFileName().GetName().ToStdString()};
        AppState::addProp(propName, propPath, propConfigPath);
        AppState::saveState();
        event.Skip();
    }, wxID_OK);
    Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_FILEPICKER_CHANGED, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { viewExisting->SetValue(true); addProp->SetValue(false); update(); }, ID_ViewExisting);
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { addProp->SetValue(true); viewExisting->SetValue(false); update(); }, ID_AddProp);
}

void Props::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    sizer->SetMinSize(wxSize(400, -1));

    auto *tabSizer{new wxBoxSizer{wxHORIZONTAL}};
    viewExisting = new wxToggleButton(this, ID_ViewExisting, "View Existing Props");
    viewExisting->SetValue(true);
    addProp = new wxToggleButton(this, ID_AddProp, "Add Prop");
    tabSizer->Add(viewExisting);
    tabSizer->Add(addProp);
    sizer->Add(tabSizer, wxSizerFlags{}.Border(wxALL, 10).Center());

    existingPanel = new wxPanel(this);
    auto *existingPanelSizer{new wxBoxSizer{wxVERTICAL}};
    existingPanelSizer->SetMinSize(wxSize{-1, 200});
    existingPanel->SetSizerAndFit(existingPanelSizer);
    sizer->Add(existingPanel, wxSizerFlags{1}.Expand());

    addPanel = new wxPanel(this);
    auto *addPanelSizer{new wxBoxSizer{wxVERTICAL}};
    choosePropText = new wxStaticText(addPanel, wxID_ANY, "ProffieOS Prop File");
    chooseProp = new wxFilePickerCtrl(addPanel, ID_Prop, wxEmptyString, "Choose Prop File to Import", "C Header (*.h)|*.h");
    choosePropConfigText = new wxStaticText(addPanel, wxID_ANY, "ProffieConfig Prop Config File");
    choosePropConfig = new wxFilePickerCtrl(addPanel, ID_Prop, wxEmptyString, "Choose Prop Config File", "ProffieConfig Data File (*.pconf)|*.pconf");
    duplicateWarning = new wxStaticText(addPanel, wxID_ANY, "Prop with same name already exists");
    fileSelectionWarning = new wxStaticText(addPanel, wxID_ANY, "Please choose prop files to import");
    addPanelSizer->Add(choosePropText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
    addPanelSizer->Add(chooseProp, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    addPanelSizer->Add(choosePropConfigText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
    addPanelSizer->Add(choosePropConfig, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    addPanelSizer->Add(duplicateWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
    addPanelSizer->Add(fileSelectionWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
    addPanelSizer->AddStretchSpacer();
    addPanelSizer->Add(new wxButton(addPanel, wxID_OK, "Ok"), wxSizerFlags().Border(wxALL, 10).Right());
    addPanel->SetSizerAndFit(addPanelSizer);
    sizer->Add(addPanel, wxSizerFlags{1}.Expand());

    SetSizerAndFit(sizer);
}

void Props::update() {
    existingPanel->Show(viewExisting->GetValue());
    addPanel->Show(addProp->GetValue());

    auto *existingPanelSizer{existingPanel->GetSizer()};
    existingPanelSizer->Clear(true);
    if (AppState::getPropFileNames().empty()) {
        existingPanelSizer->AddStretchSpacer();
        existingPanelSizer->Add(new wxStaticText(existingPanel, wxID_ANY, "No Custom Props"), wxSizerFlags{}.Center());
        existingPanelSizer->AddStretchSpacer();
    } else {
        for (const auto& propFile : AppState::getPropFileNames()) {
            auto *propSizer{new wxStaticBoxSizer(wxHORIZONTAL, existingPanel)};
            propSizer->Add(new wxStaticText(propSizer->GetStaticBox(), wxID_ANY, propFile), wxSizerFlags{}.Center());
            propSizer->AddStretchSpacer();
            auto *deleteButton{new wxButton(propSizer->GetStaticBox(), wxID_ANY, "Remove")};
            propSizer->Add(deleteButton);
            deleteButton->Bind(wxEVT_BUTTON, [this, propFile](wxCommandEvent&) {
                AppState::removeProp(propFile);
                update();
            });

            existingPanelSizer->Add(propSizer, wxSizerFlags{}.Expand().Border(wxALL, 10));
        }
    }
    existingPanelSizer->Layout();
    existingPanelSizer->Fit(existingPanel);

    auto duplicatePropFile = [&]() {
        for (const auto& propName : AppState::getPropFileNames()) {
            if (choosePropConfig->GetFileName().GetName() == (propName + ".pconf")) return true;
        }
        return false;
    }();
    auto filesSelected{chooseProp->GetFileName().FileExists() and choosePropConfig->GetFileName().FileExists()};

    duplicateWarning->Show(duplicatePropFile);
    fileSelectionWarning->Show(not filesSelected);

    FindWindowById(wxID_OK)->Enable(not duplicatePropFile and filesSelected);

    GetSizer()->Layout(); // Although linux and windows seem to work without this, macOS requires it.
    GetSizer()->Fit(this);
}
