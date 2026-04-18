#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/vecstack.hpp
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

#include "data/vector.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT VecStack {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildBase base_;

    wxOrientation orient_{wxVERTICAL};

    const data::Vector& data_;

    using Builder = std::function<DescriptorPtr(data::Model&)>;

    Builder builder_;
    DescriptorPtr separator_;
    DescriptorPtr empty_;

    DescriptorPtr operator()();
};

struct UI_EXPORT VecStack::Desc : VecStack, detail::Descriptor {
    Desc(VecStack&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui


