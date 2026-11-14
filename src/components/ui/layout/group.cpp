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

#if __WXMSW__ or __WXGTK__
#include "ui/layout/detail/panel.hpp"
#endif

using namespace pcui;

namespace {

struct Layout : detail::Window<priv::GroupBox> {
    Layout(const detail::Scaffold& scaffold, const Group& desc) {
        create(
            scaffold.childParent_,
            desc.win_.id_,
            desc.orient_,
            desc.label_,
            desc.padded_
        );

        postCreation(scaffold, desc.win_);

        detail::Scaffold child_scaffold{scaffold};
        child_scaffold.childParent_ = childParent();
        child_scaffold.sizer_ = sizer();

        for (const auto& child : desc.children_)
            sizer()->Add(child->build(child_scaffold));

        activate();
    }
};

#if __WXMSW__ or __WXGTK__
// On MSW, there's a spacing left above the static box for the label, even if
// there is no label. To avoid this, a panel w/ border is used instead, which
// achieves the same visual effect.
struct Unlabeled : detail::Window<detail::Panel> {
    Unlabeled(const detail::Scaffold& scaffold, const Group& desc) {
        create(scaffold.childParent_, desc.win_.id_, wxBORDER_STATIC);
        postCreation(scaffold, desc.win_);

        auto *sizer{new wxBoxSizer(desc.orient_)};
        auto *content_sizer{sizer};

        if (desc.padded_) {
            content_sizer = new wxBoxSizer(desc.orient_);

#           if __WXMSW__
            // The border size is hardcoded in wxWidgets
            // src/msw/statbox.cpp as CHILDREN_OFFSET.
            const auto border{5};
#           elif __WXGTK__
            // And this was arbitrality chosen in layout/priv/groupbox.cpp
            const auto border{10};
#           endif
            sizer->Add(content_sizer, 1, wxEXPAND | wxALL, border);
        }

        detail::Scaffold child_scaffold{scaffold};
        child_scaffold.childParent_ = this;
        child_scaffold.sizer_ = content_sizer;

        for (const auto& child : desc.children_)
            content_sizer->Add(child->build(child_scaffold));

        SetSizer(sizer);

        activate();
    }
};
#endif

} // namespace

DescriptorPtr Group::operator()() {
    return std::make_unique<Group::Desc>(std::move(*this));
}

Group::Desc::Desc(Group&& data) :
    Group{std::move(data)} {}

wxSizerItem *Group::Desc::build(const detail::Scaffold& scaffold) const {
#   if __WXMSW__ or __WXGTK__
    wxWindow *box{};
    if (label_.empty()) {
        box = new Unlabeled(scaffold, *this);
    } else {
        box = new Layout(scaffold, *this);
    }
#   else
    auto *box{new Layout(scaffold, *this)};
#   endif

    auto *item{new wxSizerItem(box)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Group::Desc::clone() const {
    return new Desc(*this);
}

