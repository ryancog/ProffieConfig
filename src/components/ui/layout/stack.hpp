#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/declarative/stack.hpp
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

#include "ui/declarative/descriptor.hpp"
#include "ui/declarative/general.hpp"

#include "ui_export.h"

namespace pcui::declarative {

struct UI_EXPORT Stack {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    ChildBase base_;

    wxOrientation orient_{wxVERTICAL};
    ChildList<std::unique_ptr<Descriptor>> children_;

    std::unique_ptr<Descriptor> operator()();
};

struct UI_EXPORT Stack::Desc : Stack, Descriptor {
    Desc(Stack&&);

    [[nodiscard]] wxSizerItem *build(const Scaffold&) const override;
};

} // namespace pcui::declarative

