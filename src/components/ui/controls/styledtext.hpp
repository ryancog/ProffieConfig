#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/styledtext.hpp
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

static_assert(false);
// For the presets page style editor (and maybe other things), Scintilla might
// be interesting w/ its syntax highlighting... it's got a built-in lexer for
// C++ so maybe it wouldn't be too hard to setup.
//
// It's not a very large library, and seems pretty extensively customizable.
// https://wiki.wxwidgets.org/WxStyledTextCtrl
// https://www.scintilla.org/ScintillaDoc.html

struct UI_EXPORT StyledText {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildWindowBase win_;

    std::variant<wxString, RefWrap<data::String>> data_;

    std::unique_ptr<detail::Descriptor> operator()();
};

struct UI_EXPORT StyledText::Desc : StyledText, detail::Descriptor {
    Desc(StyledText&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

} // namespace pcui


