#include "collapsible.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/collapsible.cpp
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

#include <wx/collpane.h>
#include <wx/gdicmn.h>

#include "ui/detail/helpers.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<wxCollapsiblePane> {
    Layout(const detail::Scaffold& scaffold, const Collapsible& desc) {
        long style{wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE};
        // if (not desc.autoTopLevelResize_) style |= wxCP_NO_TLW_RESIZE;

        Create(
            scaffold.childParent_,
            wxID_ANY,
            desc.showLabel_,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);

        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        auto childScaffold{scaffold};
        childScaffold.childParent_ = GetPane();
        childScaffold.sizer_ = sizer;

        sizer->Add(desc.child_->build(childScaffold));
        GetPane()->SetSizer(sizer);

        activate();
    }

    void onActivate() override {
        Window::onActivate();

        Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &Layout::onChanged, this);
    }

    void onChanged(wxCollapsiblePaneEvent& evt) {
        SetLabel(evt.GetCollapsed() ? showLabel_ : hideLabel_);
        detail::layoutAndFitFor(this);
    }

    wxString showLabel_;
    wxString hideLabel_;
};

} // namespace

DescriptorPtr Collapsible::operator()() {
    return std::make_unique<Collapsible::Desc>(std::move(*this));
}

Collapsible::Desc::Desc(Collapsible&& data) :
    Collapsible{std::move(data)} {}

wxSizerItem *Collapsible::Desc::build(const detail::Scaffold& scaffold) const {
    auto *panel{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(panel)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Collapsible::Desc::clone() const {
    return new Desc(*this);
}

