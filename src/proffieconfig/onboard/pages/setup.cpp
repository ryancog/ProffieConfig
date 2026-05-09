#include "setup.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/setup.cpp
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

#include <thread>

#include <wx/sizer.h>
#include <wx/time.h>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "utils/defer.hpp"
#include "utils/parent.hpp"
#include "versions/versions.hpp"

#include "../onboard.hpp"

#ifndef __APPLE__
#include "../../tools/arduino.hpp"
#endif

onboard::Setup::Setup() {
    mLoadingTimer = new wxTimer();
    mLoadingTimer->Start(50);
    mLoadingTimer->Bind(wxEVT_TIMER, [this](wxTimerEvent&) {
        mProgress.pulse(); 
    });
}

onboard::Setup::~Setup() {
    delete mLoadingTimer;
}

pcui::DescriptorPtr onboard::Setup::ui() {
    auto bulletString{wxString::FromUTF8("\t• ")};
    return pcui::Stack{
      .children_={
        pcui::Spacer{20}(),
        pcui::Label{
          .label_=_("Setup"),
          .font_=pcui::Font::Title,
        }(),
        pcui::Spacer{20}(),
        pcui::Label{
          .label_={
            _("Setup is about to begin.") + '\n' +
            _("An internet connection is required, and installation may take several minutes.") + 
            "\n\n" +
            bulletString + _("Core Installation") + '\n' +
            bulletString + _("ProffieOS Installation") + '\n' +
#           ifdef _WIN32
            bulletString + _("Bootloader Driver Installation") + '\n' +
#           endif
            '\n'
#           ifdef _WIN32
            + _("When the driver installation starts, you will be prompted, please follow the instructions in the new window.")
#           endif
          }
        }(),
        pcui::Spacer{10}(),
        pcui::Label{
          .win_{
            .show_=utils::parent<&Frame::mSetupPage>(*this).mPhase |
              data::logic::HasSelection{{Frame::ePhase_Setup_Done}}
          },
          .label_=_("The installation completed successfully. Press \"Next\" to continue..."),
        }(),
        pcui::Label{
          .win_={
            .show_=utils::parent<&Frame::mSetupPage>(*this).mPhase |
              data::logic::HasSelection{{Frame::ePhase_Setup_Prog}}
          },
          .label_=mStatusMessage,
        }(),
        pcui::Progress{
          .win_={
            .base_={.expand_=true},
            .maxSize_={300, -1},
            .show_=utils::parent<&Frame::mSetupPage>(*this).mPhase |
              data::logic::HasSelection{{Frame::ePhase_Setup_Prog}}
          },
          .data_=mProgress,
        }()
      }
    }();
}

void onboard::Setup::startSetup() {
    auto& phaseModel{utils::parent<&Frame::mSetupPage>(*this).mPhase};
    auto phase{data::context(phaseModel)};
    assert(
        phase.idx() == Frame::ePhase_Setup_Pre or
        phase.idx() == Frame::ePhase_Setup_Fail
    );

    phase.choose(Frame::ePhase_Setup_Prog);

    std::thread{[this, &phaseModel]() {
        bool success{false};
        defer {
            auto phase{data::context(phaseModel)};
            phase.choose(success
                ? Frame::ePhase_Setup_Done
                : Frame::ePhase_Setup_Fail
            );
        };

#       if defined(_WIN32) or defined(__linux__)
        if (not mDriverInstalled) {
            data::String::Context{mStatusMessage}.change(
                _("Installing Driver...").ToStdString()
            );

            if (arduino::runDriverInstallation()) {
                mDriverInstalled = true;
            } else {
                data::String::Context{errorMessage_}.change(
                    _("Failed to install driver.").ToStdString()
                );
                return;
            }
        }
#       endif

        if (not mOSInstalled) {
            mStatusMessage.change(_("Installing ProffieOS...").ToStdString());

            std::optional<std::string> err;

            err = versions::fetch();
            if (err) {
                errorMessage_.change(std::move(*err));
                return;
            }

            err = versions::installDefault(true);
            if (err) {
                errorMessage_.change(std::move(*err));
                return;
            }
        }

        success = true;
    }}.detach();
}

