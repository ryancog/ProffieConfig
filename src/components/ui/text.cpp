#include "text.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/text.cpp
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

wxFontInfo pcui::text::detail::StyleData::makeFont() const {
    if (const auto *ptr{std::get_if<wxFontInfo>(this)}) {
        return *ptr;
    } 

    switch (std::get<text::Style>(*this)) {
        using enum text::Style;
        case Normal:
            return wxFontInfo{};
        case Header:
            return wxFontInfo{20}.Weight(wxFONTWEIGHT_BOLD);
    }

    assert(0);
    __builtin_unreachable();
}

