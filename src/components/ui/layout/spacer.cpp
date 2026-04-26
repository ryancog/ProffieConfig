#include "spacer.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/spacer.cpp
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

#include "ui/types.hpp"

using namespace pcui;

DescriptorPtr Spacer::operator()() {
    // NOLINTNEXTLINE(performance-move-const-arg)
    return std::make_unique<Spacer::Desc>(std::move(*this));
}

Spacer::Desc::Desc(Spacer&& data) :
    // NOLINTNEXTLINE(performance-move-const-arg)
    Spacer{std::move(data)} {}

wxSizerItem *Spacer::Desc::build(const detail::Scaffold& scaffold) const {
    int orient{};

    if (auto *box{dynamic_cast<wxBoxSizer *>(scaffold.sizer_)}) {
        orient = box->GetOrientation();
    } else {
        // Only box sizer is used currently.
        assert(0);
    }

    return new wxSizerItem(
        orient == wxHORIZONTAL ? size_ : 0,
        orient == wxVERTICAL ? size_ : 0
    );
}

detail::Descriptor *Spacer::Desc::clone() const {
    return new Desc(*this);
}

DescriptorPtr StretchSpacer::operator()() {
    // NOLINTNEXTLINE(performance-move-const-arg)
    return std::make_unique<StretchSpacer::Desc>(std::move(*this));
}

StretchSpacer::Desc::Desc(StretchSpacer&& data) :
    // NOLINTNEXTLINE(performance-move-const-arg)
    StretchSpacer{std::move(data)} {}

wxSizerItem *StretchSpacer::Desc::build(
    const detail::Scaffold& scaffold
) const {
    int orient{};

    if (auto *box{dynamic_cast<wxBoxSizer *>(scaffold.sizer_)}) {
        orient = box->GetOrientation();
    } else {
        // Only box sizer is used currently.
        assert(0);
    }

    return new wxSizerItem(
        orient == wxHORIZONTAL ? size_ : 0,
        orient == wxVERTICAL ? size_ : 0,
        proportion_
    );
}

detail::Descriptor *StretchSpacer::Desc::clone() const {
    return new Desc(*this);
}

