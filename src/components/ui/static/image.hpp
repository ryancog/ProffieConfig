#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/ui/static/image.hpp
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

#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "utils/color.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Image {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildBase base_;

    struct LoadDetails {
        cstring name_;
        struct {
            int32 dim_;
            wxOrientation orient_;
        } size_;
        color::Dynamic color_;
    };

    std::variant<wxBitmap, LoadDetails> src_;

    std::unique_ptr<detail::Descriptor> operator()();
};

struct UI_EXPORT Image::Desc : Image, detail::Descriptor {
    Desc(Image&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

} // namespace pcui

