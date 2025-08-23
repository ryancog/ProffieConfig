#include "info.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/info.cpp
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

Onboard::Info::Info(wxWindow *parent) : wxPanel(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *welcomeText{Onboard::createHeader(this, _("Ready To Go!"))};

    const auto infoString{_(
        "ProffieConfig is all set up and ready to go!\n"
        "\n"
        "You can always re-run this setup from the main menu File drop-down.\n"
        "\n"
        "Additionally, the documentation linked earlier is available in the Help menu,\n"
        "and don't hesitate to reach out to for help. I'm available in a few places\n"
        "which I list on the ProffieConfig website. :)\n"
        "\n"
        "Happy Saber-ing!"
    )};
    auto *infoText{new wxStaticText(
        this,
        wxID_ANY,
        infoString
    )};

    sizer->AddSpacer(20);
    sizer->Add(welcomeText, 0);
    sizer->AddSpacer(20);
    sizer->Add(infoText, 0);
    SetSizerAndFit(sizer);
}

