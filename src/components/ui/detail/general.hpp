#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/declarative/general.hpp
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

#include <vector> // IWYU pragma: export for ChildList

#include <wx/gdicmn.h>

#include "utils/types.h"

#include "ui_export.h"

namespace pcui::declarative {

template <typename T>
struct UI_EXPORT ChildList : vector<T> {
    template <typename ...Args>
    ChildList(Args&&... args) {
        this->reserve(sizeof...(args));
        (..., this->push_back(std::forward<Args>(args)));
    }
};

/**
 * General properties for child items.
 * Most things here are only effective inside a Stack
 */
struct UI_EXPORT ChildBase {
    wxSize minSize_{wxDefaultSize};
    int32 proportion_{0};

    struct {
        int32 size_{8};
        int32 dirs_{0};
    } border_;
    bool expand_{false};

    int32 align_{wxALIGN_NOT};
};

/**
 * General properties for child window items.
 */
struct UI_EXPORT ChildWindowBase {
    wxString tooltip_;
};

} // namespace pcui::declarative

