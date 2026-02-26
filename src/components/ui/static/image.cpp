#include "image.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include "ui/priv/helpers.hpp"

#ifndef __WXOSX__
#include <wx/generic/statbmpg.h>
#else
#include <wx/statbmp.h>
#endif

using namespace pcui;

std::unique_ptr<detail::Descriptor> Image::operator()() {
    return std::make_unique<Image::Desc>(std::move(*this));
}

Image::Desc::Desc(Image&& data) :
    Image{std::move(data)} {}

wxSizerItem *Image::Desc::build(const detail::Scaffold& scaffold) const {
#   ifndef __WXOSX__
    auto *img{new wxGenericStaticBitmap(scaffold.childParent_, wxID_ANY, src_)};
#   else
    auto *img{new wxStaticBitmap(scaffold.childParent_, wxID_ANY, src_)};
#   endif
    img->SetScaleMode(wxStaticBitmapBase::Scale_AspectFill);

    auto *item{new wxSizerItem(img)};
    priv::apply(base_, item);
    return item;
}

