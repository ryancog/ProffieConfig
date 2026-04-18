#include "panel.hpp"
#include "ui/types.hpp"
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
#include <windows.h>
#endif

#include <wx/panel.h>

#include "ui/detail/datawin.hpp"

using namespace pcui;

namespace {

struct Layout : detail::DataWindow<wxPanel, data::Generic::Receiver> {
    Layout(const detail::Scaffold& scaffold, const Panel& desc) {
        Create(scaffold.childParent_);

#       ifdef _WIN32
#       ifdef __WXGTK__
        auto *hwnd{GTKGetWin32Handle()};
#       else
        auto *hwnd{GetHWND()};
#       endif

        auto exStyle{GetWindowLongA(hwnd, GWL_EXSTYLE)};
        SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
#       endif

        postCreation(scaffold, desc.win_);

        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        auto childScaffold{scaffold};
        childScaffold.childParent_ = this;
        childScaffold.sizer_ = sizer;

        sizer->Add(desc.child_->build(childScaffold));
        SetSizer(sizer);

        if (desc.data_) attach(*desc.data_);
    }

    ~Layout() override {
        detach();
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

