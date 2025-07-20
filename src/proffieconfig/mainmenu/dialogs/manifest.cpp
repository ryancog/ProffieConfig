#include "manifest.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

#include "../../core/appstate.h"

ManifestDialog::ManifestDialog(MainMenu *mainMenu) : 
    wxDialog{mainMenu, wxID_ANY, _("Set Update Channel")} {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto initialText{AppState::manifestChannel};
    constexpr cstring STABLE_CHANNEL{"stable"};
    if (initialText.empty()) initialText = STABLE_CHANNEL;
    mEntryData = std::move(initialText);
    auto *manifestEntry{new PCUI::Text(this, mEntryData)};

    sizer->Add(manifestEntry, wxSizerFlags{}.Expand().Border(wxALL, 12));
    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *cancelButton{new wxButton(this, wxID_CANCEL)};
    auto *resetButton{new wxButton(this, wxID_RESET, _("Reset to Default"))};
    auto *saveButton{new wxButton(this, wxID_SAVE)};
    saveButton->SetDefault();
    buttonSizer->Add(cancelButton);
    buttonSizer->AddStretchSpacer();
    buttonSizer->AddSpacer(20);
    buttonSizer->Add(resetButton);
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(saveButton);

    sizer->Add(buttonSizer, wxSizerFlags{}.Expand().Border(wxALL, 10));

    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        mEntryData = STABLE_CHANNEL;
    }, wxID_RESET);
    Bind(wxEVT_BUTTON, [this, manifestEntry](wxCommandEvent&) {
        if (static_cast<string>(mEntryData) == STABLE_CHANNEL) AppState::manifestChannel.clear();
        else AppState::manifestChannel = mEntryData;

        AppState::saveState();
        Close();
    }, wxID_SAVE);

    mEntryData.setUpdateHandler([this, saveButton]() {
        saveButton->Enable(not static_cast<string>(mEntryData).empty());
    });

    SetSizerAndFit(sizer);
}


