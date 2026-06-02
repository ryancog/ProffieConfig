#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/editor/special/splitvisualizer.hpp
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

#include "config/blades/ws281x.hpp"
#include "data/base/models/selector.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

struct SplitVisualizer {
    struct Desc;

    pcui::detail::ChildBase base_;

    config::blades::WS281X& blade_;
    data::base::Selector& subSel_;

    pcui::DescriptorPtr operator()();
};

struct SplitVisualizer::Desc : SplitVisualizer, pcui::detail::Descriptor {
    Desc(SplitVisualizer&&);

    [[nodiscard]] wxSizerItem *build(
        const pcui::detail::Scaffold&
    ) const override;

    [[nodiscard]] Descriptor *clone() const override;
};

