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

#include <thread>

#include <wx/sizer.h>
#include <wx/time.h>

#include "../onboard.h"
#include "../../tools/arduino.h"

#include "utils/defer.h"
#include "versions/versions.h"

Onboard::Setup::Setup(wxWindow* parent) : wxPanel(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *title{Onboard::createHeader(this, _("Setup"))};

    auto bulletString{wxString::FromUTF8("\t• ")};
    auto descriptionString{
        _("Setup is about to begin.") + '\n' +
        _("An internet connection is required, and installation may take several minutes.") + 
        "\n\n" +
        bulletString + _("Core Installation") + '\n' +
        bulletString + _("ProffieOS Installation") + '\n' +
#       ifdef __WINDOWS__
        bulletString + _("Bootloader Driver Installation") + '\n' +
#       endif
        '\n'
#       ifdef __WINDOWS__
        + _("When the driver installation starts, you will be prompted, please follow the instructions in the new window.")
#       endif
    };

    auto *description{new wxStaticText(this, wxID_ANY, descriptionString)};
    mPressNextMessage = new wxStaticText(this, wxID_ANY, _("Press \"Next\" to begin installation.\n"));
    mDoneMessage = new wxStaticText(this, wxID_ANY, _("The installation completed successfully. Press \"Next\" to continue..."));
    mDoneMessage->Hide();

    mLoadingBar = new wxGauge(this, wxID_ANY, 50, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH);
    loadingText = new wxStaticText(this, wxID_ANY, wxEmptyString);
    mLoadingBar->SetMinSize({200, -1});
    mLoadingBar->Hide();
    loadingText->Hide();

    sizer->AddSpacer(20);
    sizer->Add(title);
    sizer->AddSpacer(20);
    sizer->Add(description);
    sizer->AddSpacer(10);
    sizer->Add(mPressNextMessage);
    sizer->Add(mDoneMessage);
    sizer->Add(loadingText);
    sizer->Add(mLoadingBar);
    SetSizerAndFit(sizer);

    Bind(wxEVT_TIMER, [this](wxTimerEvent&) { 
        mLoadingBar->Pulse(); 
    });
}

void Onboard::Setup::startSetup() {
    static bool inProgress{false};
    assert(not inProgress);

    mLoadingTimer = new wxTimer(this);
    mLoadingTimer->Start(50);
    mPressNextMessage->Hide();
    loadingText->Show();
    mLoadingBar->Show();

    std::thread{[this]() {
        Defer defer{[]() { inProgress = false; }};

        if (not mCoreInstalled) {
            statusMessage = "Installing Core...";
            notifier.notify(ID_STATUS);
            if (Arduino::ensureDefaultCoreInstalled()) {
                mCoreInstalled = true;
            } else {
                errorMessage = _("Failed to install core.");
                notifier.notify(ID_FAILED);
                return;
            }
        }

#       if defined(__WINDOWS__) or defined(__linux__)
        if (not mDriverInstalled) {
            statusMessage = "Installing Driver...";
            notifier.notify(ID_STATUS);
            if (Arduino::runDriverInstallation()) {
                mDriverInstalled = true;
            } else {
                errorMessage = _("Failed to install driver.");
                notifier.notify(ID_FAILED);
                return;
            }
        }
#       endif

        if (not mOSInstalled) {
            statusMessage = "Installing ProffieOS...";
            notifier.notify(ID_STATUS);
            auto err{Versions::resetToDefault(true)};
            if (err) {
                errorMessage = *err;
                notifier.notify(ID_FAILED);
                return;
            }
        }

        isDone = true;
        notifier.notify(ID_DONE);
    }}.detach();
}

void Onboard::Setup::finishSetup(bool done) {
    loadingText->Hide();
    mLoadingBar->Hide();
    if (done) {
        mDoneMessage->Show();
    } else {
        mPressNextMessage->Show();
    }

    delete mLoadingTimer;

    Layout();
    Fit();
}


