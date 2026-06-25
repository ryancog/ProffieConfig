#include "onboard.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/onboard.cpp
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

#include <utility>

#include <wx/menu.h>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/bitmap.hpp"
#include "ui/controls/button.hpp"
#include "ui/layout/panel.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/image.hpp"
#include "ui/values.hpp"
#include "utils/paths.hpp"

#include "../core/state.hpp"
#include "../mainmenu/mainmenu.hpp"

onboard::Frame* onboard::Frame::instance{nullptr};

onboard::Frame::Frame() : 
    pcui::Frame(
        nullptr,
        state::eID_Onboard,
        _("ProffieConfig First-Time Setup"),
        wxSYSTEM_MENU | wxCLOSE_BOX | wxMINIMIZE_BOX |
        wxCAPTION | wxCLIP_CHILDREN
    ),
    mSetupPage(*this) {

    { auto phase{data::context(mPhase)};
        phase.update(ePhase_Max);
        phase.choose(ePhase_Welcome);
    }

    pcui::build(this, ui());

    createMenuBar();
    bindEvents();

    activate();

    CentreOnScreen();
    Show(true);
}

onboard::Frame::~Frame() {
    pcui::cripple(this);
    instance = nullptr;
}

void onboard::Frame::onActivate() {
    onPhase();
}

pcui::DescriptorPtr onboard::Frame::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={900, 430},
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .children_={
        pcui::Stack{
          .base_={.expand_=true, .proportion_=1},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Spacer{10}(),
            pcui::Image{
              .win_={.maxSize_={256, 256}},
              .src_=pcui::Bitmap("icon"),
            }(),
            pcui::Spacer{10}(),
            pcui::Panel{
              .win_={
                .base_={.proportion_=1},
                .show_=mPhase | data::logic::HasSelection{{ePhase_Welcome}}
              },
              .child_=mWelcomePage.ui(),
            }(),
            pcui::Panel{
              .win_={
                .base_={.proportion_=1},
                .show_=mPhase | data::logic::HasSelection{{
                    ePhase_Setup_Pre,
                    ePhase_Setup_Prog,
                    ePhase_Setup_Fail,
                    ePhase_Setup_Done,
                  }}
              },
              .child_=mSetupPage.ui(),
            }(),
            pcui::Panel{
              .win_={
                .base_={.proportion_=1},
                .show_=mPhase | data::logic::HasSelection{{ePhase_Info}}
              },
              .child_=mInfoPage.ui(),
            }(),
            pcui::Spacer{10}(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Divider{.base_={.expand_=true}}(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={
            .expand_=true,
          },
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .win_={
                .enable_=mMayCancel | data::logic::IsSet{},
              },
              .label_=_("Cancel"),
              .func_=[this] {
                  auto res{pcui::showMessage(
                      _("Are you sure you want to cancel setup?"),
                      {
                          .caption_=_("Exit ProffieConfig"),
                          .style_=wxYES_NO | wxNO_DEFAULT | wxCENTER,
                          .parent_=this
                      }
                  )};

                  if (res == wxYES) Close();
              }
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Button{
              .win_={
                .show_=not (mPhase | data::logic::HasSelection{{
                    ePhase_Welcome,
                    ePhase_Setup_Done,
                    ePhase_Info
                  }}),
                .enable_=mMaySkip | data::logic::IsSet{},
              },
              .label_=_("Skip"),
              .func_=[this] {
                  auto res{pcui::showMessage(
                      _("Skipping will leave ProffieConfig and your computer unprepared.") +
                      "\n\n" +
                      _("You should only do this if you know what you are doing!"),
                      {
                          .caption_=_("Skip Setup?"),
                          .style_=wxYES_NO | wxNO_DEFAULT,
                          .parent_=this
                      }
                  )};
                  if (res == wxYES) {
                      mPhase.choose(ePhase_Info);
                  }
              }
            }(),
            pcui::StretchSpacer{}(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Button{
              .win_={
                .enable_=mMayGoBack | data::logic::IsSet{},
              },
              .label_=_("Back"),
              .func_=[this] {
                  auto phase{data::context(mPhase)};

                  switch (static_cast<Phase>(phase.idx())) {
                      case ePhase_Setup_Pre:
                      case ePhase_Setup_Fail:
                      case ePhase_Setup_Done:
                          phase.choose(ePhase_Welcome);
                          break;
                      case ePhase_Info:
                          phase.choose(ePhase_Setup_Done);
                          break;
                      case ePhase_Welcome:
                      case ePhase_Setup_Prog:
                      case ePhase_Max:
                          std::unreachable();
                          break;
                  }
              },
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Button{
              .label_=mNextButton,
              .func_=[this] {
                  auto phase{data::context(mPhase)};

                  switch (static_cast<Phase>(phase.idx())) {
                      case ePhase_Welcome:
                          if (mSetupDone) {
                              phase.choose(ePhase_Setup_Done);
                          } else phase.choose(ePhase_Setup_Pre);
                          break;
                      case ePhase_Setup_Pre:
                      case ePhase_Setup_Fail:
                          mSetupPage.startSetup();
                          break;
                      case ePhase_Setup_Done:
                          phase.choose(ePhase_Info);
                          break;
                      case ePhase_Info:
                          state::doneWithFirstRun = true;
                          state::saveState();
                          Close(true);
                          MainMenu::instance = new MainMenu;
                          break;
                      case ePhase_Setup_Prog:
                      case ePhase_Max:
                          std::unreachable();
                  }
              }
            }(),
          }
        }(),
      }
    }();
}

void onboard::Frame::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent &event) {
        if (event.CanVeto() and state::doneWithFirstRun) {
            MainMenu::instance = new MainMenu;
        }

        event.Skip();
    });

    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        wxLaunchDefaultApplication(paths::logDir().string());
    }, wxID_FILE1);

    static const auto phaseTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&Frame::onPhase>();
        return table;
    }()};
    observeWith(mPhase, phaseTable);
}

void onboard::Frame::createMenuBar() {
    auto *menubar{new wxMenuBar};

    auto *file{new wxMenu};
    file->Append(wxID_FILE1, _("Show Logs..."));

    menubar->Append(file, _("&File"));
    appendDefaultMenuItems(menubar);

    SetMenuBar(menubar);
}

void onboard::Frame::onPhase() {
    auto phase{data::context(mPhase)};

    auto mayCancel{data::context(mMayCancel)};
    auto maySkip{data::context(mMaySkip)};
    auto mayGoBack{data::context(mMayGoBack)};
    auto nextButton{data::context(mNextButton)};

    switch (static_cast<Phase>(phase.idx())) {
        case ePhase_Welcome:
        case ePhase_Setup_Prog:
        case ePhase_Setup_Done:
            nextButton.change(_("Next").utf8_string());
            break;
        case ePhase_Setup_Pre:
            nextButton.change(_("Run Setup").utf8_string());
            break;
        case ePhase_Setup_Fail:
            nextButton.change(_("Try Again").utf8_string());
            break;
        case ePhase_Info:
            nextButton.change(_("Finish").utf8_string());
            break;
        case ePhase_Max:
            nextButton.change("Where Are You?");
            break;
    }

    mayCancel.set(phase.idx() != ePhase_Setup_Prog);
    maySkip.set(phase.idx() != ePhase_Setup_Prog);
    mayGoBack.set(
        phase.idx() != ePhase_Welcome and phase.idx() != ePhase_Setup_Prog
    );
    nextButton.enable(phase.idx() != ePhase_Setup_Prog);

    mSetupDone |= phase.idx() == ePhase_Setup_Done;

    if (phase.idx() == ePhase_Setup_Fail) {
        CallAfter([this] {
            pcui::showMessage(
                _("Dependency installation failed, please try again.") +
                "\n\n" +
                data::context(mSetupPage.errorMessage_).val(),
                {
                    .caption_=_("Installation Failure"),
                    .style_=wxOK | wxCENTER,
                    .parent_=this
                }
            );
        });
    }
}

