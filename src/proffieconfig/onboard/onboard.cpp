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
#include "../mainmenu/mainmenu.h"

#include "ui/message.h"
#include "ui/notifier.h"
#include "utils/image.h"
#include "ui/plaque.h"

constexpr cstring NEXT_STR{wxTRANSLATE("Next")};
constexpr cstring FINISH_STR{wxTRANSLATE("Finish")};
constexpr cstring RUN_SETUP_STR{wxTRANSLATE("Run Setup")};

Onboard::Frame* Onboard::Frame::instance{nullptr};

Onboard::Frame::Frame() : 
    PCUI::Frame(
        nullptr,
        wxID_ANY,
        _("ProffieConfig First-Time Setup"),
        wxDefaultPosition,
        wxDefaultSize,
        wxSYSTEM_MENU | wxCLOSE_BOX | wxMINIMIZE_BOX | wxCAPTION | wxCLIP_CHILDREN
    ) {
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
    auto *skip{new wxButton(this, ID_Skip, _("Skip"))};
    skip->Hide();
    auto *cancel{new wxButton(this, wxID_CANCEL)};
    auto *back{new wxButton(this, wxID_BACKWARD)};
    back->Disable();
    auto *next{new wxButton(this, ID_Next, wxGetTranslation(NEXT_STR))};
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(cancel);
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(skip);
    buttonSizer->AddStretchSpacer();
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(back);
    buttonSizer->AddSpacer(10);
    buttonSizer->Add(next);
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

    bindEvents();
    NotifyReceiver::create(this, mSetupPage->notifier);
    initializeNotifier();
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
                if (AppState::doneWithFirstRun) {
                    MainMenu::instance = new MainMenu;
                }
                event.Veto();
                return;
            }
        }
        event.Skip();
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Close();
    }, wxID_CANCEL);
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(Misc::EVT_MSGBOX, [&](Misc::MessageBoxEvent& evt) {
        PCUI::showMessage(evt.message, evt.caption, evt.style, this);
    }, wxID_ANY);

    // TODO: Make this button handling sane.
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto res{PCUI::showMessage(
            _("Skipping will leave ProffieConfig and your computer unprepared.\nYou should only do this if you know what you are doing!"),
            _("Skip Setup?"),
            wxYES_NO | wxNO_DEFAULT,
            this
        )};
        if (res == wxYES) {
            mSetupPage->Hide();
            mInfoPage->Show();
            FindWindow(ID_Next)->SetLabel(wxGetTranslation(FINISH_STR));
            FindWindow(ID_Skip)->Hide();
            Layout();
            Fit();
        }
    }, ID_Skip);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (mInfoPage->IsShown()) {
            mInfoPage->Hide();
            mSetupPage->Show();
            if (mSetupPage->isDone) {
                FindWindow(ID_Next)->SetLabel(wxGetTranslation(NEXT_STR));
                FindWindow(ID_Skip)->Hide();
            } else {
                FindWindow(ID_Next)->SetLabel(wxGetTranslation(RUN_SETUP_STR));
                FindWindow(ID_Skip)->Show();
            }
        } else if (mSetupPage->IsShown()) {
            mSetupPage->Hide();
            mWelcomePage->Show();
            FindWindow(wxID_BACKWARD)->Disable();
            FindWindow(ID_Skip)->Hide();
            FindWindow(ID_Next)->SetLabel(wxGetTranslation(NEXT_STR));
        }

        Layout();
        Fit();
    }, wxID_BACKWARD);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (mWelcomePage->IsShown()) {
            mWelcomePage->Hide();
            mSetupPage->Show();
            FindWindow(ID_Next)->SetLabel(wxGetTranslation(RUN_SETUP_STR));
            if (not mSetupPage->isDone) FindWindow(ID_Skip)->Show();
        } else if (mSetupPage->IsShown()) {
            if (not mSetupPage->isDone) {
                FindWindow(ID_Next)->Disable();
                FindWindow(ID_Skip)->Disable();
                FindWindow(wxID_BACKWARD)->Disable();
                FindWindow(wxID_CANCEL)->Disable();

                mSetupPage->startSetup();
            } else {
                mSetupPage->Hide();
                mInfoPage->Show();
                FindWindow(ID_Next)->SetLabel(wxGetTranslation(FINISH_STR));
            }
        } else if (mInfoPage->IsShown()) {
            AppState::doneWithFirstRun = true;
            AppState::saveState();
            Close(true);
            MainMenu::instance = new MainMenu;
        }

        FindWindow(wxID_BACKWARD)->Enable();
        Layout();
        Fit();
    }, ID_Next);
}

void Onboard::Frame::handleNotification(uint32 id) {
    if (id == Onboard::Setup::ID_DONE or id == Onboard::Setup::ID_FAILED) {
        mSetupPage->finishSetup(id == Onboard::Setup::ID_DONE);

        FindWindow(ID_Next)->Enable();
        FindWindow(ID_Skip)->Enable();
        FindWindow(wxID_BACKWARD)->Enable();
        FindWindow(wxID_CANCEL)->Enable();
    }

    if (id == Onboard::Setup::ID_DONE) {
        FindWindow(ID_Next)->SetLabel(wxGetTranslation(NEXT_STR));
        FindWindow(ID_Skip)->Hide();
    } else if (id == Onboard::Setup::ID_FAILED) {
        FindWindow(ID_Next)->SetLabel(_("Try Again"));
        
        PCUI::showMessage(
            _("Dependency installation failed, please try again.") + "\n\n"
            + mSetupPage->errorMessage,
            _("Installation Failure"),
            wxOK | wxCENTER,
            this
        );
    } else if (id == Onboard::Setup::ID_STATUS) {
        mSetupPage->loadingText->SetLabelText(mSetupPage->statusMessage);
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
