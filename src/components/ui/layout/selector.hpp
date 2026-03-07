#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/selector.hpp
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

#include "data/selector.hpp"
#include "ui/detail/builder.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Selector {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildBase base_;
    detail::ChildWindowBase win_;

    data::Selector& data_;

    detail::DescBuilder builder_;

    std::unique_ptr<detail::Descriptor> operator()();
};

struct UI_EXPORT Selector::Desc : Selector, detail::Descriptor {
    Desc(Selector&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

} // namespace pcui

