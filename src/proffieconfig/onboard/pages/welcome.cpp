#include "welcome.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/welcome.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <wx/hyperlink.h>
#include <wx/sizer.h>

#include "../onboard.h"
#include "utils/paths.h"

Onboard::Welcome::Welcome(wxWindow* parent) : wxPanel(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *welcomeText{Onboard::createHeader(this, wxString::Format(_("Welcome to ProffieConfig %s!"), wxSTRINGIZE(VERSION)))};

    /*
     * So, cool story, the WIDE window bug on macOS has to do with double
     * newline characters. E.g. "\n\n".
     *
     * Why? Don't Know.
     */
    const auto infoString1{_(
        "Thank you for trying out ProffieConfig, the all-in-one proffieboard management utility!"
    )};
    const auto infoString2{_(
        "Online guides are available at the link below:"
    )};
    const auto infoString3{_(
        "To start, ProffieConfig needs to do some setup.\n"
        "Press \"Next\" when you're ready to continue, and we'll get started!"
    )};
    auto *infoText1{new wxStaticText(
        this,
        wxID_ANY,
        infoString1,
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    )};
    auto *infoText2{new wxStaticText(
        this,
        wxID_ANY,
        infoString2,
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    )};
    auto *docLink{new wxHyperlinkCtrl(
        this,
        wxID_ANY,
        _("Guides And Documentation"),
        Paths::website() + "/guides"
    )};
    auto *infoText3{new wxStaticText(
        this,
        wxID_ANY,
        infoString3,
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    )};

    sizer->AddSpacer(20);
    sizer->Add(welcomeText, 0, wxCENTER);
    sizer->AddSpacer(40);
    sizer->Add(infoText1, 0, wxCENTER);
    sizer->AddSpacer(20);
    sizer->Add(infoText2, 0, wxCENTER);
    sizer->Add(docLink, 0, wxCENTER);
    sizer->AddSpacer(20);
    sizer->Add(infoText3, 0, wxCENTER);
    SetSizerAndFit(sizer);
}
