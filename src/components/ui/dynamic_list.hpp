#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/dynamic_list.hpp
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

#include <vector>

#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT DynamicList : std::vector<DescriptorPtr> {
    DynamicList(const DynamicList&) = default;
    DynamicList(DynamicList&&) = default;
    DynamicList& operator=(const DynamicList&) = default;
    DynamicList& operator=(DynamicList&&) = default;

    template <typename ...Args>
    DynamicList(Args&&... args) {
        // This reserve is inaccurate
        reserve(sizeof...(args));
        (..., add(std::forward<Args>(args)));
    }

    // Rather than having a DynamicList&&, use a template so that implicit
    // conversion, which may select the templated ctor and spiral into
    // unbounded recursion, cannot occur.
    void add(std::same_as<DynamicList> auto&& v) {
        for (auto& elem : v) push_back(std::move(elem));
    }

    void add(DescriptorPtr&& d) {
        push_back(std::move(d));
    }
};

} // namespace pcui

