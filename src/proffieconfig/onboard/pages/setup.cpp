#include "setup.hpp"
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

#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "utils/defer.hpp"
#include "versions/versions.hpp"

pcui::DescriptorPtr onboard::Setup::ui() {
    auto bulletString{wxString::FromUTF8("\t• ")};
    return pcui::Stack{
      .children_={
        pcui::Spacer{20}(),
        pcui::Label{
          .label_=_("Setup"),
          .style_=pcui::text::Style::Header,
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
            .win_={
                .show_={
                    not data::logic::adapt(isDone_) and
                    not data::logic::adapt(inProgress_)
                },
            },
            .label_=_("Press \"Next\" to begin installation.\n"),
        }(),
        pcui::Label{
            .win_{
                .show_=data::logic::adapt(isDone_),
            },
            .label_=_("The installation completed successfully. Press \"Next\" to continue..."),
        }(),
        pcui::Label{
            .win_={.show_=data::logic::adapt(inProgress_)},
            .label_=statusMessage_,
        }(),
        pcui::Progress{
            .win_={.show_=data::logic::adapt(inProgress_)},
            .data_=progress_,
        }()
      }
    }();
}

onboard::Setup::Setup() {
    mLoadingTimer = new wxTimer();
    mLoadingTimer->Start(50);
    mLoadingTimer->Bind(wxEVT_TIMER, [this](wxTimerEvent&) {
        progress_.pulse(); 
    });
}

void onboard::Setup::startSetup() {
    data::Bool::Context inProgress{inProgress_};
    assert(not inProgress.val());

    inProgress.set(true);

    std::thread{[this]() {
        defer { 
            data::Bool::Context inProgress{inProgress_};
            inProgress.set(false);
        };

#       if defined(_WIN32) or defined(__linux__)
        if (not mDriverInstalled) {
            data::String::Context{statusMessage_}.change(
                _("Installing Driver...").ToStdString()
            );

            if (Arduino::runDriverInstallation()) {
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
            data::String::Context{statusMessage_}.change(
                _("Installing ProffieOS...").ToStdString()
            );

            std::optional<std::string> err;

            err = versions::fetch();
            if (err) {
                data::String::Context{errorMessage_}.change(std::move(*err));
                return;
            }

            err = versions::installDefault(true);
            if (err) {
                data::String::Context{errorMessage_}.change(std::move(*err));
                return;
            }
        }

        data::Bool::Context{isDone_}.set(true);
    }}.detach();
}

void onboard::Setup::finishSetup(bool done) {
    /*
    Layout();
    Fit();
    */
}

