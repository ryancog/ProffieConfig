#include "group.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/ui/layout/group.cpp
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

#include <wx/panel.h>
#include <wx/statbox.h>

#include "ui/layout/priv/groupbox.hpp"
#include "ui/priv/helpers.hpp"

using namespace pcui;

std::unique_ptr<detail::Descriptor> Group::operator()() {
    return std::make_unique<Group::Desc>(std::move(*this));
}

Group::Desc::Desc(Group&& data) :
    Group{std::move(data)} {}

wxSizerItem *Group::Desc::build(const detail::Scaffold& scaffold) const {
    auto *box{new priv::GroupBox(
        orient_, scaffold.childParent_, label_
    )};

    detail::Scaffold childScaffold{
        .childParent_ = box->childParent()
    };

    for (const auto& child : children_) {
        box->sizer()->Add(child->build(childScaffold));
    }

    auto *item{new wxSizerItem(box)};
    priv::apply(base_, item);

    return item;
}

