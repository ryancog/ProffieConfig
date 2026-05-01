#include "split.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/split.cpp
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
#include <wx/splitter.h>

#include "ui/build.hpp"
#include "ui/detail/window.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<wxSplitterWindow> {
    Layout(const detail::Scaffold& scaffold, const Split& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize,
            wxSP_3DSASH | wxSP_LIVE_UPDATE
        );

        postCreation(scaffold, desc.win_);

        auto *pane1{new wxPanel(this)};
        auto *pane2{new wxPanel(this)};

        build(pane1, desc.child1_);
        build(pane2, desc.child2_);

        // I don't support this for now, as the child ownership is unclear and
        // it's not useful without a controller
        assert(desc.minPaneSize_ > 0);
        SetMinimumPaneSize(static_cast<int32>(desc.minPaneSize_));

        // This follows the wxSizer idea of horizontal and vertical, describing
        // the axis along which windows are placed rather than the axis of the
        // sash, for consistency, which is opposite the naming of the
        // functions.
        if (desc.orient_ == wxHORIZONTAL) {
            SplitVertically(pane1, pane2);
        } else SplitHorizontally(pane1, pane2);

        activate();
    }
};

} // namespace

DescriptorPtr Split::operator()() {
    return std::make_unique<Split::Desc>(std::move(*this));
}

Split::Desc::Desc(Split&& data) :
    Split{std::move(data)} {}

wxSizerItem *Split::Desc::build(const detail::Scaffold& scaffold) const {
    auto *split{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(split)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Split::Desc::clone() const {
    return new Desc(*this);
}

