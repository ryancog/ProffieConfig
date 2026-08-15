#include "data_context.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/data_context.cpp
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

#include <wx/thread.h>
#include <wx/evtloop.h>
#include <wx/window.h>

#include "ui/dialogs/progress.hpp"

void pcui::priv::doGuiDataLock(const data::base::Model& model) {
    if (not wxIsMainThread()) {
        model.lock();
        return;
    }

    if (model.tryLock(std::chrono::milliseconds(20)))
        return;

    auto *prog{new pcui::ProgressDialog(
        wxGetActiveWindow(),
        _("Data In Use")
    )};
    prog->pulse(_("An operation is in progress, please wait..."));

    { 
        wxGUIEventLoop loop;
        wxEventLoopActivator activator(&loop);

        auto lastPulse{std::chrono::steady_clock::now()};
        while (loop.Dispatch()) {
            if (model.tryLock())
                break;

            auto now{std::chrono::steady_clock::now()};
            if (now - lastPulse > std::chrono::milliseconds(50))
                prog->pulse();
        }
    }

    prog->finish(false);
}

