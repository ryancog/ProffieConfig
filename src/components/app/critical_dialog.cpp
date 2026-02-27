#include "critical_dialog.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/app/critical_dialog.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

#include "app/app.hpp"
#include "utils/paths.hpp"

namespace {

enum {
    eID_Ok = 2,
    eID_Logs
};

} // namespace

app::CriticalDialog::CriticalDialog(
    const wxString& error, const wxString& detail
) : wxDialog(
        nullptr,
        wxID_ANY,
        getName() + " Has Crashed",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxCENTER | wxRESIZE_BORDER
    ) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *errorMessage{new wxStaticText(
        this,
        wxID_ANY,
        error,
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    )};
    sizer->Add(errorMessage, wxSizerFlags(0).Border(wxALL, 10));

    if (not detail.IsEmpty()) {
        auto *detailMessage{new wxTextCtrl(
            this,
            wxID_ANY,
            detail,
            wxDefaultPosition,
            wxDefaultSize,
            wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP
        )};
        detailMessage->SetFont(wxFontInfo{}.Family(wxFONTFAMILY_TELETYPE));
        sizer->Add(detailMessage, 1, wxEXPAND);
    }

    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *logButton{new wxButton(this, eID_Logs, "Show Log Folder")};
    buttonSizer->Add(logButton, wxSizerFlags(1).Expand().Border(wxALL, 5));

    auto *okButton{new wxButton(this, eID_Ok, "Ok")};
    buttonSizer->Add(
        okButton,
        wxSizerFlags(1).Expand().Border(wxRIGHT | wxTOP | wxBOTTOM, 5)
    );

    sizer->Add(buttonSizer, wxSizerFlags().Expand());
    SetSizerAndFit(sizer);

    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        Close();
    }, eID_Ok);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        wxLaunchDefaultApplication(paths::logDir().native());
    }, eID_Logs);
}

