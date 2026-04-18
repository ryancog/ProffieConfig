#include "scrolled.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/scrolled.cpp
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

#include <wx/scrolwin.h>

#include "ui/detail/window.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<wxScrolledWindow> {
    Layout(const detail::Scaffold& scaffold, const Scrolled& desc) {
        Create(scaffold.childParent_, wxID_ANY);

        postCreation(scaffold, desc.win_);
        SetScrollRate(desc.scrollRate_.x_, desc.scrollRate_.y_);

        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        auto childScaffold{scaffold};
        childScaffold.childParent_ = this;
        childScaffold.scrolled_ = this;
        childScaffold.sizer_ = sizer;

        sizer->Add(desc.child_->build(childScaffold));
        SetSizer(sizer);
    }

    void updateSizes() override {
        Window::updateSizes();
        
        if (GetSizer() == nullptr) return;

        // Do not reset min size, it was recalc'd in WinBase::updateSizes()
        auto bestSize{GetBestSize()};

        int scrollX{};
        int scrollY{};
        GetScrollPixelsPerUnit(&scrollX, &scrollY);

        const auto virtSize{GetSizer()->CalcMin()};

        if (scrollX == -1) {
            bestSize.SetWidth(std::max(
                bestSize.GetWidth(),
                virtSize.GetWidth()
            ));
        }

        if (scrollY == -1) {
            bestSize.SetHeight(std::max(
                bestSize.GetHeight(),
                virtSize.GetHeight()
            ));
        }

        SetMinSize(bestSize);
    }
};

} // namespace

DescriptorPtr Scrolled::operator()() {
    return std::make_unique<Scrolled::Desc>(std::move(*this));
}

Scrolled::Desc::Desc(Scrolled&& data) :
    Scrolled{std::move(data)} {}

wxSizerItem *Scrolled::Desc::build(const detail::Scaffold& scaffold) const {
    auto *win{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(win)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Scrolled::Desc::clone() const {
    return new Desc(*this);
}

