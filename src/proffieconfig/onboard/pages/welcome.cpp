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

#include <wx/sizer.h>

#include "../onboard.h"

Onboard::Welcome::Welcome(wxWindow* parent) : wxPanel(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *welcomeText{Onboard::createHeader(this, wxString::Format(_("Welcome to ProffieConfig %s!"), wxSTRINGIZE(VERSION)))};

    const auto infoString{_(
        "ProffieConfig is an All-in-One utility for managing your Proffieboard.\n"
        "Links to documentation can be found in the application under Help->Documentation...\n"
        "\n"
        "This wizard will guide you through first-time setup and usage of ProffieConfig.\n"
        "\n\n"
        "Press \"Next\" when you're ready to continue, and we'll get started!"
    )};
    auto *infoText{new wxStaticText(this, wxID_ANY, infoString)};

    sizer->Add(welcomeText, wxSizerFlags(0).Center());
    sizer->AddSpacer(40);
    sizer->Add(infoText, wxSizerFlags(0).Center());
    SetSizerAndFit(sizer);
}
