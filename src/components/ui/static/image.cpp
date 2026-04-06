#include "image.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/ui/static/image.cpp
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

#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/rawbmp.h>
#include <wx/sizer.h>

#ifndef __WXOSX__
#include <wx/generic/statbmpg.h>
#else
#include <wx/statbmp.h>
#endif

#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

#ifdef __WXOSX__
using Widget = wxStaticBitmap;
#else
using Widget = wxGenericStaticBitmap;
#endif

struct Static : detail::DataWindow<Widget, data::Generic::Receiver> {
    Static(
        const detail::Scaffold& scaffold,
        const Image& desc
    ) {
        Create(scaffold.childParent_, wxID_ANY, {});

        postCreation(scaffold, desc.win_);

        bmp_ = desc.src_;

        const auto updateBitmap{[this]() {
            SetBitmap(bmp_.realize());
        }};

        Bind(
            wxEVT_SYS_COLOUR_CHANGED,
            [updateBitmap](wxSysColourChangedEvent& evt) {
                evt.Skip();
                updateBitmap();
            }
        );

        updateBitmap();

        SetScaleMode(desc.scale_);

        if (desc.data_) attach(*desc.data_);
    }

    Bitmap bmp_;
};

} // namespace

DescriptorPtr Image::operator()() {
    return std::make_unique<Image::Desc>(std::move(*this));
}

Image::Desc::Desc(Image&& data) :
    Image{std::move(data)} {}

wxSizerItem *Image::Desc::build(const detail::Scaffold& scaffold) const {
    auto *img{new Static(scaffold, *this)};
    auto *item{new wxSizerItem(img)};
    detail::apply(win_.base_, item);
    return item;
}

