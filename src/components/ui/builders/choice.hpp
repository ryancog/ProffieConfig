#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/builders/choice.hpp
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

#include <functional>

#include "data/base/models/choice.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui::builders {

struct UI_EXPORT Choice {
    struct Desc;

    const data::base::Choice& data_;

    using DescBuilder = DescriptorPtr(int32);
    std::function<DescBuilder> builder_;

    DescriptorPtr operator()();
};

struct UI_EXPORT Choice::Desc : Choice, detail::Descriptor {
    Desc(Choice&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui::builders


