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

#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/log.h>
#include <wx/rawbmp.h>
#include <wx/sizer.h>

#ifndef __WXOSX__
#include <wx/generic/statbmpg.h>
#else
#include <wx/statbmp.h>
#endif

#include "ui/priv/helpers.hpp"
#include "utils/paths.hpp"


using namespace pcui;

namespace {

const wxColour DARK_BLUE{39, 74, 114};
const wxColour LIGHT_BLUE{31, 99, 168};

void generateMissingBMP(wxBitmap&, const wxSize& = wxDefaultSize);

// std::unordered_map<string, wxBitmap> bmps;
wxBitmap loadPNG(cstring);
wxBitmap loadPNG(cstring, int32, wxOrientation, const wxColour&);

} // namespace

std::unique_ptr<detail::Descriptor> Image::operator()() {
    return std::make_unique<Image::Desc>(std::move(*this));
}

Image::Desc::Desc(Image&& data) :
    Image{std::move(data)} {}

wxSizerItem *Image::Desc::build(const detail::Scaffold& scaffold) const {
    wxBitmap bmp;
    if (const auto *ptr{std::get_if<wxBitmap>(&src_)}) {
        bmp = *ptr;
    } else if (const auto *ptr{std::get_if<LoadDetails>(&src_)}) {
        if (ptr->size_.dim_ == -1) {
            bmp = loadPNG(ptr->name_);
        } else {
            bmp = loadPNG(
                ptr->name_,
                ptr->size_.dim_,
                ptr->size_.orient_,
                ptr->color_.color()
            );
        }
    } else assert(0);

#   ifndef __WXOSX__
    auto *img{new wxGenericStaticBitmap(scaffold.childParent_, wxID_ANY, bmp)};
#   else
    auto *img{new wxStaticBitmap(scaffold.childParent_, wxID_ANY, bmp)};
#   endif
    img->SetScaleMode(wxStaticBitmapBase::Scale_AspectFill);

    auto *item{new wxSizerItem(img)};
    priv::apply(base_, item);
    return item;
}

namespace {

wxBitmap loadPNG(cstring name) {
    // auto bmpIt{bmps.find(name)};
    // if (bmpIt != bmps.end()) return bmpIt->second;

    auto pngPath{paths::resourceDir() / "icons"};
    pngPath /= name;
    pngPath += ".png";

    // std::cout << "Loading PNG \"" << name << "\" from \"" << pngPath.native() << '"' << std::endl;
    wxBitmap bitmap;
    {
        wxLogNull noErrors;
        bitmap.LoadFile(pngPath.native(), wxBITMAP_TYPE_PNG);
        if (not bitmap.IsOk()) {
            generateMissingBMP(bitmap);
            return bitmap;
        }
    }
    bitmap.SetScaleFactor(wxDisplay{0U}.GetScaleFactor());

    // bmps.emplace(name, bitmap);
    return bitmap;
}

wxBitmap loadPNG(cstring name, int32 dim, wxOrientation orient, const wxColour& color) {
    auto pngPath{paths::resourceDir() / "icons"};
    pngPath /= name;
    pngPath += ".png";

    wxBitmap bitmap;
    {
        wxLogNull noErrors;
        bitmap.LoadFile(pngPath.native(), wxBITMAP_TYPE_PNG);
        bitmap.UseAlpha();
        if (not bitmap.IsOk()) {
            generateMissingBMP(bitmap);
            return bitmap;
        }
    }

    float64 scaler{};
    if (orient == wxHORIZONTAL) {
        scaler = bitmap.GetLogicalWidth() / dim;
    } else {
        scaler = bitmap.GetLogicalHeight() / dim;
    }

#   ifdef _WIN32
    auto img{bitmap.ConvertToImage()};
    bitmap = img.Scale(
        static_cast<int32>(bitmap.GetWidth() / scaler),
        static_cast<int32>(bitmap.GetHeight() / scaler)
    );
#   else
    bitmap.SetScaleFactor(bitmap.GetScaleFactor() * scaler);
#   endif

    if (color.IsOk()) {
        wxAlphaPixelData data{bitmap};
        if (not data) return bitmap;

        auto iter{data.GetPixels()};
        for (auto yIdx{0}; yIdx < data.GetHeight(); ++yIdx) {
            wxAlphaPixelData::Iterator rowStart = iter;

            for (auto xIdx{0}; xIdx < data.GetWidth(); ++xIdx, ++iter) {
                if (iter.Alpha() > 0) {
#                   ifdef _WIN32
                    // Idk man windows is funky with its bitmaps.
                    auto alphaScale{static_cast<float64>(iter.Alpha()) / 0xFF};
                    iter.Red() = static_cast<uint8>(color.Red() * alphaScale);
                    iter.Green() = static_cast<uint8>(color.Green() * alphaScale);
                    iter.Blue() = static_cast<uint8>(color.Blue() * alphaScale);
#                   else
                    iter.Red() = color.Red();
                    iter.Green() = color.Green();
                    iter.Blue() = color.Blue();
#                   endif
                }
            }

            iter = rowStart;
            iter.OffsetY(data, 1);
        }
    }

    return bitmap;
}

void generateMissingBMP(wxBitmap& bmp, const wxSize& size) {
    int32 dimension{};
    if (size.GetX() == -1 and size.GetY() == -1) dimension = 32;
    else dimension = std::max(size.x, size.y);

    bmp.Create(dimension, dimension, 32);
    bmp.UseAlpha(true);
    wxMemoryDC bmpDC{bmp};
    bmpDC.SetFont(wxFont{
        dimension,
        wxFONTFAMILY_MODERN,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD
    });

    bmpDC.SetBrush(*wxTRANSPARENT_BRUSH);
    bmpDC.DrawRectangle(0, 0, dimension, dimension);
    bmpDC.SetBrush(wxBrush(LIGHT_BLUE));
    bmpDC.DrawRoundedRectangle(0, 0, dimension, dimension, 4);

    auto extent{bmpDC.GetTextExtent("?")};
    extent /= 2;
    dimension /= 2;
    bmpDC.SetTextForeground(DARK_BLUE);
    bmpDC.DrawText("?", dimension - extent.x, dimension - extent.y);
}

} // namespace


