#include "onboard.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024-2025 Ryan Ogurek

#include <wx/bitmap.h>
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

const wxEventTypeTag<Onboard::UpdateEvent> OnboardFrame::EVT_UPDATE(wxNewEventType());

OnboardFrame* OnboardFrame::instance{nullptr};
OnboardFrame::OnboardFrame() : 
    PCUI::Frame(nullptr, wxID_ANY, _("ProffieConfig First-Time Setup"), wxDefaultPosition, wxDefaultSize, wxSYSTEM_MENU | wxCLOSE_BOX | wxMINIMIZE_BOX | wxCAPTION | wxCLIP_CHILDREN) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *contentSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *icon{PCUI::createStaticImage(this, wxID_ANY, Image::loadPNG("icon"))};
    icon->SetMaxSize({256, 256});

    contentSizer->Add(icon, wxSizerFlags(0).Border(wxALL, 10));
    mWelcomePage = new Onboard::Welcome(this);
    mDependencyPage = new Onboard::DependencyInstall(this);
    mDependencyPage->Hide();
    mOverviewPage = new Onboard::Overview(this);
    mOverviewPage->Hide();
    contentSizer->Add(mWelcomePage, wxSizerFlags(1).Expand());
    contentSizer->Add(mDependencyPage, wxSizerFlags(1).Expand());
    contentSizer->Add(mOverviewPage, wxSizerFlags(1).Expand());

    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    mSkipIntro = new wxButton(this, ID_SkipIntro, _("Skip Introduction"));
    mSkipIntro->Hide();
    mSkipInstall = new wxButton(this, ID_SkipInstall, _("Skip Dependency Installation"));
    mSkipInstall->Hide();
    mNext = new wxButton(this, ID_Next, _("Next >"));
    mCancel = new wxButton(this, ID_Cancel, _("Cancel"));
    buttonSizer->Add(mSkipIntro, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10));
    buttonSizer->Add(mSkipInstall, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10));
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(mNext, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));
    buttonSizer->Add(mCancel, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));

    sizer->Add(contentSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));
    sizer->Add(new wxStaticLine(this, wxID_ANY), wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 10));
    sizer->Add(buttonSizer, wxSizerFlags(0).Expand());

    sizer->SetMinSize(wxSize(900, 430));
    SetSizerAndFit(sizer);

    bindEvents(); // Do this first so children can bind their events to parent state.

    CentreOnScreen();
    Show(true);
}

OnboardFrame::~OnboardFrame() {
    instance = nullptr;
}

void OnboardFrame::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) {
        if (event.CanVeto() && PCUI::showMessage(_("Are you sure you want to cancel setup?"), _("Exit ProffieConfig"), wxYES_NO | wxNO_DEFAULT | wxCENTER, this) == wxNO) {
            event.Veto();
            return;
        }
        event.Skip();
        if (AppState::doneWithFirstRun) MainMenu::instance = new MainMenu();
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Close(); }, ID_Cancel);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent &) {
        if (PCUI::showMessage(
                _("Are you sure you want to skip the Introduction?") + 
                "\n\n" +
                _("The introduction covers all the basics and usage of ProffieConfig."),
                _("Skip Introduction"), wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION,
                this) == wxYES
            ) {
            wxPostEvent(GetEventHandler(), wxCommandEvent(wxEVT_BUTTON, ID_Next));
        }
    },
    ID_SkipIntro);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        mDependencyPage->completedInstall = true;
        wxPostEvent(GetEventHandler(), wxCommandEvent(wxEVT_BUTTON, ID_Next));
    }, ID_SkipInstall);
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
        const auto& msgEvent{static_cast<Misc::MessageBoxEvent&>(event)};
        PCUI::showMessage(msgEvent.message, msgEvent.caption, msgEvent.style, this);
    },
    wxID_ANY);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        if (mWelcomePage->IsShown()) {
            mWelcomePage->Hide();
            mDependencyPage->Show();
        } else if (mDependencyPage->IsShown()) {
            if (!mDependencyPage->completedInstall) dependencyInstall(event);
            else {
                mDependencyPage->Hide();
                mOverviewPage->Show();
                mOverviewPage->prepare();
            }
        } else if (mOverviewPage->IsShown()) {
            AppState::doneWithFirstRun = true;
            AppState::saveState();
            Close(true);
        }
        update();
    }, ID_Next);

    Bind(EVT_UPDATE, [&](Onboard::UpdateEvent& event) {
        Enable();
        mDependencyPage->loadingBar->Hide();
        mDependencyPage->barPulser->Stop();

        if (event.succeeded) {
            mDependencyPage->description->Hide();
            mDependencyPage->doneMessage->Show();
            mDependencyPage->Layout();

            mDependencyPage->completedInstall = true;
        } else {
            mDependencyPage->pressNext->Show();
            mDependencyPage->Layout();

            PCUI::showMessage(_("Dependency installation failed, please try again.") + "\n\n" + event.message.ToStdString(), _("Installation Failure"), wxOK | wxCENTER, this);
        }
    }, ID_DependencyInstall);
}

void OnboardFrame::update() {
    if (mOverviewPage->IsShown()) {
        mSkipIntro->Show();
        mSkipInstall->Hide();
        mNext->Enable(mOverviewPage->isDone);
        mNext->SetLabel(_("Finish"));
    } else if (mDependencyPage->IsShown() && AppState::doneWithFirstRun) {
        mSkipInstall->Show();
    } else {
        mSkipIntro->Hide();
        mSkipInstall->Hide();
        mNext->Enable();
        mNext->SetLabel(_("Next >"));
    }

    Layout();
    Fit();
}

void OnboardFrame::dependencyInstall(wxCommandEvent&) {
    Disable();
    mDependencyPage->pressNext->Hide();
    mDependencyPage->loadingBar->Show();
    mDependencyPage->Layout();

    mDependencyPage->barPulser->Start(50);
}

wxStaticText* OnboardFrame::createHeader(wxWindow* parent, const wxString& text) {
    auto *header{new wxStaticText(parent, wxID_ANY, text)};
    auto font = header->GetFont();
    font.MakeBold();
    font.SetPointSize(20);
    header->SetFont(font);

    return header;
}
