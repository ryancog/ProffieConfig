#include "busy.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/busy.cpp
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

#include <cctype>
#include <map>

#include <wx/app.h>
#include <wx/busycursor.h>
#include <wx/thread.h>

#include "utils/types.hpp"

using namespace pcui;

namespace {

struct WindowData {
    size count_;
    wxCursor prevCursor_;
};

std::map<wxWindow *, WindowData> datas;

void beginBusy(wxWindow *);
void endBusy(wxWindow *);

} // namespace

BusyTracker::BusyTracker(wxWindow *win) : mWindow{win} {
    assert(mWindow != nullptr);

    if (wxIsMainThread()) {
        beginBusy(mWindow);
    } else {
        // Do not capture `this`, it will likely die (stack created or in
        // thread closure data) before the call completes.
        mWindow->CallAfter([win=mWindow] { beginBusy(win); });
    }
}

BusyTracker::~BusyTracker() {
    if (wxIsMainThread()) {
        endBusy(mWindow);
    } else {
        mWindow->CallAfter([win=mWindow] { endBusy(win); });
    }
}

BusyTracker::BusyTracker(const BusyTracker& other) : BusyTracker(other.mWindow) {}

namespace {

void beginBusy(wxWindow *win) {
    auto iter{datas.find(win)};

    if (iter != datas.end()) {
        ++iter->second.count_;
        return;
    }

    auto& data{datas[win]};

    data.count_ = 1;
    data.prevCursor_ = win->GetCursor();

    win->SetCursor(wxCURSOR_WAIT);

    // TLWs must be explicitly disabled, wxWidgets won't do it.
    // Prevent input which, e.g. when data is locked for the operation,
    // could cause the application to hang until the operation is complete.
    for (auto *child : win->GetChildren()) {
        if (child->IsTopLevel())
            child->Disable();
    }
}

void endBusy(wxWindow *win) {
    auto iter{datas.find(win)};
    auto& data{iter->second};

    --data.count_;

    if (data.count_ > 0)
        return;

    win->SetCursor(data.prevCursor_);

    for (auto *child : win->GetChildren()) {
        if (child->IsTopLevel())
            child->Enable();
    }

    datas.erase(iter);
}

} // namespace

