#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/filepicker.hpp
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

#include "data/string.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT FilePicker {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildWindowBase win_;

    data::String& data_;

    /**
     * Message displayed in the dialog.
     */
    wxString message_;

    /**
     * wxWidgets picker wildcard to filter files.
     */
    wxString wildcard_;

    struct Open {
        /**
         * A UX option: The user can only select existing files in the dialog.
         * The control may still have an invalid/nonexistent file for other
         * reasons, however.
         */
        bool mustExist_{true};
    };

    struct Save {
        /**
         * Prompt the user if they try to overwrite a file.
         */
        bool confirmOverwrite_{true};
    };

    std::variant<Open, Save> mode_;

    DescriptorPtr operator()();
};

struct UI_EXPORT FilePicker::Desc : FilePicker, detail::Descriptor {
    Desc(FilePicker&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui

