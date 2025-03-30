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

Props::Props(MainMenu* parent) : wxDialog(parent, wxID_ANY, _("Prop Files"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), mParent(parent) {
  createUI();
  bindEvents();
  update();

  FindWindowById(wxID_OK)->Disable();
}

void Props::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        const auto propConfigPath{mChoosePropConfig->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propPath{mChooseProp->GetFileName().GetAbsolutePath().ToStdString()};
        const auto propName{mChoosePropConfig->GetFileName().GetName().ToStdString()};
        AppState::addProp(propName, propPath, propConfigPath);
        AppState::saveState();
        event.Skip();
    }, wxID_OK);
    Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_FILEPICKER_CHANGED, [&](wxCommandEvent&) { update(); });
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { mViewExisting->SetValue(true); mAddProp->SetValue(false); update(); }, ID_ViewExisting);
    Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent&) { mAddProp->SetValue(true); mViewExisting->SetValue(false); update(); }, ID_AddProp);
}

void Props::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    sizer->SetMinSize(wxSize(400, -1));

    auto *tabSizer{new wxBoxSizer{wxHORIZONTAL}};
    mViewExisting = new wxToggleButton(this, ID_ViewExisting, _("View Existing Props"));
    mViewExisting->SetValue(true);
    mAddProp = new wxToggleButton(this, ID_AddProp, _("Add Prop"));
    tabSizer->Add(mViewExisting);
    tabSizer->Add(mAddProp);
    sizer->Add(tabSizer, wxSizerFlags{}.Border(wxALL, 10).Center());

    mExistingPanel = new wxPanel(this);
    auto *existingPanelSizer{new wxBoxSizer{wxVERTICAL}};
    existingPanelSizer->SetMinSize(wxSize{-1, 200});
    mExistingPanel->SetSizerAndFit(existingPanelSizer);
    sizer->Add(mExistingPanel, wxSizerFlags{1}.Expand());

    mAddPanel = new wxPanel(this);
    auto *addPanelSizer{new wxBoxSizer{wxVERTICAL}};
    mChoosePropText = new wxStaticText(mAddPanel, wxID_ANY, _("ProffieOS Prop File"));
    mChooseProp = new wxFilePickerCtrl(mAddPanel, ID_Prop, wxEmptyString, _("Choose Prop File to Import"), "C Header (*.h)|*.h");
    mChoosePropConfigText = new wxStaticText(mAddPanel, wxID_ANY, _("ProffieConfig Prop Config File"));
    mChoosePropConfig = new wxFilePickerCtrl(mAddPanel, ID_Prop, wxEmptyString, _("Choose Prop Config File"), "ProffieConfig Data File (*.pconf)|*.pconf");
    mDuplicateWarning = new wxStaticText(mAddPanel, wxID_ANY, _("Prop with same name already exists"));
    mFileSelectionWarning = new wxStaticText(mAddPanel, wxID_ANY, _("Please choose prop files to import"));
    addPanelSizer->Add(mChoosePropText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
    addPanelSizer->Add(mChooseProp, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    addPanelSizer->Add(mChoosePropConfigText, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).DoubleBorder(wxLEFT));
    addPanelSizer->Add(mChoosePropConfig, wxSizerFlags(0).Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    addPanelSizer->Add(mDuplicateWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
    addPanelSizer->Add(mFileSelectionWarning, wxSizerFlags(0).Right().Border(wxRIGHT, 10));
    addPanelSizer->AddStretchSpacer();
    addPanelSizer->Add(new wxButton(mAddPanel, wxID_OK, _("Ok")), wxSizerFlags().Border(wxALL, 10).Right());
    mAddPanel->SetSizerAndFit(addPanelSizer);
    sizer->Add(mAddPanel, wxSizerFlags{1}.Expand());

    SetSizerAndFit(sizer);
}

void Props::update() {
    mExistingPanel->Show(mViewExisting->GetValue());
    mAddPanel->Show(mAddProp->GetValue());

    auto *existingPanelSizer{mExistingPanel->GetSizer()};
    existingPanelSizer->Clear(true);
    if (AppState::getPropFileNames().empty()) {
        existingPanelSizer->AddStretchSpacer();
        existingPanelSizer->Add(new wxStaticText(mExistingPanel, wxID_ANY, _("No Custom Props")), wxSizerFlags{}.Center());
        existingPanelSizer->AddStretchSpacer();
    } else {
        for (const auto& propFile : AppState::getPropFileNames()) {
            auto *propSizer{new wxStaticBoxSizer(wxHORIZONTAL, mExistingPanel)};
            propSizer->Add(new wxStaticText(propSizer->GetStaticBox(), wxID_ANY, propFile), wxSizerFlags{}.Center());
            propSizer->AddStretchSpacer();
            auto *deleteButton{new wxButton(propSizer->GetStaticBox(), wxID_ANY, _("Remove"))};
            propSizer->Add(deleteButton);
            deleteButton->Bind(wxEVT_BUTTON, [this, propFile](wxCommandEvent&) {
                AppState::removeProp(propFile);
                update();
            });

            existingPanelSizer->Add(propSizer, wxSizerFlags{}.Expand().Border(wxALL, 10));
        }
    }
    existingPanelSizer->Layout();
    existingPanelSizer->Fit(mExistingPanel);

    auto duplicatePropFile = [&]() {
        for (const auto& propName : AppState::getPropFileNames()) {
            if (mChoosePropConfig->GetFileName().GetName() == (propName + ".pconf")) return true;
        }
        return false;
    }();
    auto filesSelected{mChooseProp->GetFileName().FileExists() and mChoosePropConfig->GetFileName().FileExists()};

    mDuplicateWarning->Show(duplicatePropFile);
    mFileSelectionWarning->Show(not filesSelected);

    FindWindowById(wxID_OK)->Enable(not duplicatePropFile and filesSelected);

    GetSizer()->Layout(); // Although linux and windows seem to work without this, macOS requires it.
    GetSizer()->Fit(this);
}
