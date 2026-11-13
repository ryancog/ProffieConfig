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

#include <wx/gdicmn.h>
#include <wx/scrolwin.h>

#include "ui/detail/window.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<wxScrolledWindow> {
    Layout(const detail::Scaffold& scaffold, const Scrolled& desc) {
        Create(scaffold.childParent_, desc.win_.id_);

        postCreation(scaffold, desc.win_);
        SetScrollRate(desc.scrollRate_.x_, desc.scrollRate_.y_);

        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        auto child_scaffold{scaffold};
        child_scaffold.childParent_ = this;
        child_scaffold.scrolled_ = this;
        child_scaffold.sizer_ = sizer;

        sizer->Add(desc.child_->build(child_scaffold));
        SetSizer(sizer);

        activate();
    }

    void updateSizes() override {
        Window::updateSizes();
        
        if (GetSizer() == nullptr) return;

        // Do not reset min size, it was recalc'd in WinBase::updateSizes()
        auto best_size{GetBestSize()};

        int scroll_x{};
        int scroll_y{};
        GetScrollPixelsPerUnit(&scroll_x, &scroll_y);

        const auto virt_size{GetSizer()->CalcMin()};

        if (scroll_x == -1) {
            best_size.SetWidth(std::max(
                best_size.GetWidth(),
                virt_size.GetWidth()
            ));
        }

        if (scroll_y == -1) {
            best_size.SetHeight(std::max(
                best_size.GetHeight(),
                virt_size.GetHeight()
            ));
        }

        SetMinSize(best_size);
    }
};

} // namespace

DescriptorPtr Scrolled::operator()() {
    // Scrolled will by default return a non-zero minsize which adds a space
    // when the sizer computes its size. This is annoying when trying to
    // manually add the actual content minsize.
    //
    // Also be sure not to set it if the scrollrate is unset, to not break the
    // custom sizing in updateSizes().
    auto& min_size{win_.base_.minSize_};
    if (min_size.GetWidth() == -1 and scrollRate_.x_ != -1)
        min_size.SetWidth(0);
    if (min_size.GetHeight() == -1 and scrollRate_.y_ != -1)
        min_size.SetHeight(0);

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

