#include "setup.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/setup.cpp
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

Onboard::Setup::Setup(wxWindow* parent) : wxPanel(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *title{Onboard::createHeader(this, _("Dependency Installation"))};

    auto bulletString{wxString::FromUTF8("\t• ")};
    auto descriptionString{
        _("In order to continue, ProffieConfig needs to do some setup.") + '\n' +
        _("This will involve the following:") +
        "\n\n" +
        bulletString + _("ProffieOS Download") + '\n' +
#       ifdef __WINDOWS__
        bulletString + _("Proffieboard Driver Installation") + '\n' +
#       endif
        '\n' +
        _("An internet connection is required, and installation may take several minutes.")
#       ifdef __WINDOWS__
        + '\n' + _("When the driver installation starts, you will be prompted, please follow the instructions in the new window.")
#       endif
    };

    auto *description{new wxStaticText(this, wxID_ANY, descriptionString)};
    auto *pressNext{new wxStaticText(this, wxID_ANY, _("Press \"Next\" to begin installation.\n"))};
    auto *doneMessage{new wxStaticText(this, wxID_ANY, _("The installation completed successfully. Press \"Next\" to continue..."))};
    doneMessage->Hide();

    auto *loadingBar{new wxGauge(this, wxID_ANY, 50, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH)};
    loadingBar->Hide();

    sizer->Add(title);
    sizer->AddSpacer(40);
    sizer->Add(description);
    sizer->AddSpacer(10);
    sizer->Add(pressNext);
    sizer->Add(loadingBar);
    sizer->Add(doneMessage);
    SetSizerAndFit(sizer);

    Bind(wxEVT_IDLE, [&](wxIdleEvent&) { 
        if (loadingBar->IsShown()) loadingBar->Pulse(); 
    });
}

void Onboard::Setup::startSetup() {

}

