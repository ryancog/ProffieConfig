#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/busy.hpp
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

#include <wx/window.h>

#include "ui_export.h"

namespace pcui {

/**
 * RAII object for handling busy indication.
 * Safe to have cross thread boundaries (including non-main)
 */
struct UI_EXPORT BusyTracker {
    BusyTracker(wxWindow *);
    ~BusyTracker();

    BusyTracker(const BusyTracker&);

    BusyTracker(BusyTracker&&) = delete;
    BusyTracker& operator=(const BusyTracker&) = delete;
    BusyTracker& operator=(BusyTracker&&) = delete;

private:
    wxWindow *mWindow;
};

} // namespace pcui

