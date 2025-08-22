#include "onboard.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/onboard/onboard.cpp
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

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/statline.h>
#include <wx/event.h>
#include <wx/statbmp.h>

#include "../core/utilities/progress.h"
#include "../core/utilities/misc.h"
#include "../core/appstate.h"

#include "ui/message.h"
#include "utils/image.h"
#include "ui/plaque.h"

constexpr cstring NEXT_STR{wxTRANSLATE("Next >")};

Onboard::Frame* Onboard::Frame::instance{nullptr};

Onboard::Frame::Frame() : 
    PCUI::Frame(nullptr, wxID_ANY, _("ProffieConfig First-Time Setup"), wxDefaultPosition, wxDefaultSize, wxSYSTEM_MENU | wxCLOSE_BOX | wxMINIMIZE_BOX | wxCAPTION | wxCLIP_CHILDREN) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *contentSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *icon{PCUI::createStaticImage(this, wxID_ANY, Image::loadPNG("icon"))};
    icon->SetMaxSize({256, 256});

    contentSizer->AddSpacer(10);
    contentSizer->Add(icon);
    mWelcomePage = new Onboard::Welcome(this);
    mSetupPage = new Onboard::Setup(this);
    mInfoPage = new Onboard::Info(this);
    contentSizer->Add(mWelcomePage, wxSizerFlags(1).Expand());
    contentSizer->Add(mSetupPage, wxSizerFlags(1).Expand());
    contentSizer->Add(mInfoPage, wxSizerFlags(1).Expand());
    contentSizer->AddSpacer(10);

    mSetupPage->Hide();
    mInfoPage->Hide();

    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *next{new wxButton(this, ID_Next, wxGetTranslation(NEXT_STR))};
    auto *cancel{new wxButton(this, ID_Cancel, _("Cancel"))};
    buttonSizer->AddStretchSpacer();
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(next);
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(cancel);
    buttonSizer->AddSpacer(10);

    sizer->AddSpacer(10);
    sizer->Add(contentSizer, 1, wxEXPAND);
    sizer->AddSpacer(10);
    sizer->Add(
        new wxStaticLine(this, wxID_ANY),
        wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)
    );
    sizer->AddSpacer(10);
    sizer->Add(buttonSizer, 0, wxEXPAND);
    sizer->AddSpacer(10);

    sizer->SetMinSize(wxSize(900, 430));
    SetSizerAndFit(sizer);

    bindEvents(); // Do this first so children can bind their events to parent state.

    CentreOnScreen();
    Show(true);
}

Onboard::Frame::~Frame() {
    instance = nullptr;
}

void Onboard::Frame::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) {
        if (event.CanVeto()) {
            auto res{PCUI::showMessage(
                _("Are you sure you want to cancel setup?"),
                _("Exit ProffieConfig"),
                wxYES_NO | wxNO_DEFAULT | wxCENTER,
                this
            )};
            if (res != wxYES) {
                event.Veto();
                return;
            }
        }
        event.Skip();
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Close(); }, ID_Cancel);
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(Misc::EVT_MSGBOX, [&](Misc::MessageBoxEvent& evt) {
        PCUI::showMessage(evt.message, evt.caption, evt.style, this);
    }, wxID_ANY);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (mWelcomePage->IsShown()) {
            mWelcomePage->Hide();
            mSetupPage->Show();
        } else if (mSetupPage->IsShown()) {
            if (not mSetupPage->isDone) {
                mSetupPage->startSetup();
                FindWindow(ID_Next)->SetLabel(_("Run Setup"));
                Disable();
            } else {
                mSetupPage->Hide();
                mInfoPage->Show();
                FindWindow(ID_Next)->SetLabel(_("Finish"));
            }
        } else if (mInfoPage->IsShown()) {
            AppState::doneWithFirstRun = true;
            AppState::saveState();
            Close(true);
        }

        Layout();
        Fit();
    }, ID_Next);
}

void Onboard::Frame::handleNotification(uint32 id) {
    if (id == Onboard::Setup::ID_DONE) {
        Enable();
        FindWindow(ID_Next)->SetLabel(wxGetTranslation(NEXT_STR));
    } else if (id == Onboard::Setup::ID_FAILED) {
        FindWindow(ID_Next)->SetLabel(_("Try Again"));
        Enable();
        PCUI::showMessage(
            _("Dependency installation failed, please try again."),
            _("Installation Failure"),
            wxOK | wxCENTER,
            this
        );
    }
}

wxStaticText *Onboard::createHeader(wxWindow* parent, const wxString& text) {
    auto *header{new wxStaticText(parent, wxID_ANY, text)};
    auto font = header->GetFont();
    font.MakeBold();
    font.SetPointSize(20);
    header->SetFont(font);

    return header;
}
