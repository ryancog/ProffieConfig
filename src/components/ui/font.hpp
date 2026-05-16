#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/font.hpp
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

#include <variant>

#include <wx/font.h>

#include "ui_export.h"

namespace pcui {

enum class Font {
    Normal,
    Monospace,
    Title,
    Header,
};

namespace detail {

struct UI_EXPORT FontData : std::variant<Font, wxFont> {
    using variant::variant;

    FontData() : variant{Font::Normal} {}

    [[nodiscard]] wxFont makeFont() const;
};

} // namespace detail

UI_EXPORT wxFont operator-(Font);

} // namespace pcui

