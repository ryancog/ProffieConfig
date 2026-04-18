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

#include "ui/detail/scaffold.hpp"
#include "ui/layout/priv/groupbox.hpp"
#include "ui/detail/window.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<priv::GroupBox> {
    Layout(const detail::Scaffold& scaffold, const Group& desc) {
        create(
            scaffold.childParent_,
            desc.orient_,
            desc.label_
        );

        postCreation(scaffold, desc.win_);

        detail::Scaffold childScaffold{scaffold};
        childScaffold.childParent_ = childParent();
        childScaffold.sizer_ = sizer();

        for (const auto& child : desc.children_) {
            sizer()->Add(child->build(childScaffold));
        }
    }

};

} // namespace

DescriptorPtr Group::operator()() {
    return std::make_unique<Group::Desc>(std::move(*this));
}

Group::Desc::Desc(Group&& data) :
    Group{std::move(data)} {}

wxSizerItem *Group::Desc::build(const detail::Scaffold& scaffold) const {
    auto *box{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(box)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Group::Desc::clone() const {
    return new Desc(*this);
}

