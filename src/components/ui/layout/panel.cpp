#include "panel.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/panel.cpp
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <wx/panel.h>

#include "ui/detail/window.hpp"
#include "ui/layout/detail/panel.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<detail::Panel> {
    Layout(const detail::Scaffold& scaffold, const pcui::Panel& desc) {
        create(scaffold.childParent_, desc.win_.id_);
        postCreation(scaffold, desc.win_);

        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        auto childScaffold{scaffold};
        childScaffold.childParent_ = this;
        childScaffold.sizer_ = sizer;

        sizer->Add(desc.child_->build(childScaffold));
        SetSizer(sizer);

        activate();
    }
};

} // namespace

DescriptorPtr Panel::operator()() {
    return std::make_unique<Panel::Desc>(std::move(*this));
}

Panel::Desc::Desc(Panel&& data) :
    Panel{std::move(data)} {}

wxSizerItem *Panel::Desc::build(const detail::Scaffold& scaffold) const {
    auto *panel{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(panel)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Panel::Desc::clone() const {
    return new Desc(*this);
}

