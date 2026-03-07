#include "onboard.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
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

#include "ui/controls/button.hpp"
#include "ui/layout/panel.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/misc/message.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/image.hpp"

#include "../core/state.hpp"

namespace {

constexpr cstring NEXT_STR{wxTRANSLATE("Next")};
constexpr cstring FINISH_STR{wxTRANSLATE("Finish")};
constexpr cstring RUN_SETUP_STR{wxTRANSLATE("Run Setup")};

} // namespace

onboard::Frame* onboard::Frame::instance{nullptr};

onboard::Frame::Frame() : 
    pcui::Frame(
        nullptr,
        wxID_ANY,
        _("ProffieConfig First-Time Setup"),
        wxDefaultPosition,
        wxDefaultSize,
        wxSYSTEM_MENU | wxCLOSE_BOX | wxMINIMIZE_BOX | wxCAPTION | wxCLIP_CHILDREN
    ) {

    bindEvents();
    CentreOnScreen();
    Show(true);
}

onboard::Frame::~Frame() {
    instance = nullptr;
}

pcui::DescriptorPtr onboard::Frame::ui() {
    return pcui::Stack{
      .base_={.minSize_={900, 430},},
      .children_={
        pcui::Spacer{10}(),
        pcui::Stack{
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Spacer{10}(),
            pcui::Image{
              .src_=pcui::Image::LoadDetails{
                .name_="icon",
                .size_={.dim_=256},
              },
            }(),
            pcui::Spacer{10}(),
            pcui::Panel{
                .win_={.show_={}},
                .child_=mWelcomePage.ui(),
            }(),
            pcui::Panel{
                .win_={.show_={}},
                .child_=mSetupPage.ui(),
            }(),
            pcui::Panel{
                .win_={.show_={}},
                .child_=mInfoPage.ui(),
            }(),
            pcui::Spacer{10}(),
          }
        }(),
        pcui::Spacer{10}(),
        pcui::Divider{
            .base_={
                .border_={.size_=10, .dirs_=wxLEFT | wxRIGHT,},
                .expand_=true
            },
        }(),
        pcui::Spacer{10}(),
        pcui::Stack{
            .orient_=wxHORIZONTAL,
            .children_={
              pcui::Spacer{10}(),
              pcui::Button{
                .label_=_("Cancel"),
              }(),
              pcui::Spacer{10}(),
              pcui::Button{
                .label_=_("Skip"),
              }(),
              pcui::StretchSpacer{}(),
              pcui::Spacer{10}(),
              pcui::Button{
                .label_=_("Back"),
              }(),
              pcui::Spacer{10}(),
              pcui::Button{
                .label_=wxGetTranslation(NEXT_STR),
              }(),
              pcui::Spacer{10}(),
            }
        }(),
        pcui::Spacer{10}(),
      }
    }();
}

void onboard::Frame::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) {
        if (event.CanVeto()) {
            auto res{pcui::showMessage(
                _("Are you sure you want to cancel setup?"),
                _("Exit ProffieConfig"),
                wxYES_NO | wxNO_DEFAULT | wxCENTER,
                this
            )};
            if (res != wxYES) {
                event.Veto();
                return;
            }

            if (state::doneWithFirstRun) {
                // MainMenu::instance = new MainMenu;
            }
        }
        event.Skip();
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        Close();
    }, wxID_CANCEL);
    // TODO: Make this button handling sane.
    /*
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto res{pcui::showMessage(
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
    */
}

/*
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
        
        pcui::showMessage(
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
*/

