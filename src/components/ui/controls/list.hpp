#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/list.hpp
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

#include "data/base/models/number.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT List {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildWindowBase win_;

    /**
     * If a count is provided, no headers are shown.
     */
    std::variant<size, std::vector<wxString>> columns_;

    using Rows = std::variant<size, RefWrap<const data::base::Integer>>;
    Rows rows_;

    using Label = std::variant<wxString, RefWrap<const data::base::String>>;

    /**
     * Provide the label to use for given row, column
     */
    using Labeler = std::function<Label(size, size)>;

    Labeler labeler_;

    DescriptorPtr operator()();
};

struct UI_EXPORT List::Desc : List, detail::Descriptor {
    Desc(List&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui

