#include "stack.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/stack.cpp
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

#include <wx/sizer.h>
#include <wx/statbox.h>

using namespace pcui;

DescriptorPtr Stack::operator()() {
    return std::make_unique<Stack::Desc>(std::move(*this));
}

Stack::Desc::Desc(Stack&& data) :
    Stack{std::move(data)} {}

wxSizerItem *Stack::Desc::build(const detail::Scaffold& scaffold) const {
    auto *sizer{new wxBoxSizer(orient_)};

    auto childScaffold{scaffold};
    childScaffold.sizer_ = sizer;

    for (const auto& child : children_) {
        sizer->Add(child->build(childScaffold));
    }

    auto *item{new wxSizerItem(sizer)};
    detail::apply(base_, item);

    return item;
}

detail::Descriptor *Stack::Desc::clone() const {
    return new Desc(*this);
}

