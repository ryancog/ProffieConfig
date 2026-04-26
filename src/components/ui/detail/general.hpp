#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/general.hpp
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

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>

#include "data/logic/logic.hpp"
#include "utils/types.hpp"

#include "ui_export.h"

namespace pcui::detail {

/**
 * General properties for child items.
 * Most things here are only effective inside a Stack
 */
struct UI_EXPORT ChildBase {
    wxSize minSize_{wxDefaultSize};

    bool expand_{false};
    int32 proportion_{0};

    struct {
        int32 size_{8};
        int32 dirs_{0};
    } border_;

    wxAlignment align_{wxALIGN_NOT};
};

/**
 * General properties for child window items.
 */
struct UI_EXPORT ChildWindowBase {
    ChildBase base_;

    wxSize maxSize_{wxDefaultSize};

    data::logic::Holder show_;
    data::logic::Holder enable_;

    wxString tooltip_;

    /**
     * Make this window be focused by default.
     */
    bool focus_{false};

    /**
     * Forward mouse events upwards. I.e. skip them on this window.
     * Useful inside scrolled.
     */
    bool forwardMouse_{false};
};

/**
 * Apply general ChildBase attributes to a sizer item.
 */
UI_EXPORT void apply(const ChildBase&, wxSizerItem *);

} // namespace pcui::detail

