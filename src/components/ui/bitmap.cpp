#include "bitmap.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/bitmap.cpp
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

#include "utils/paths.hpp"

using namespace pcui;

namespace {

const wxColour DARK_BLUE{39, 74, 114};
const wxColour LIGHT_BLUE{31, 99, 168};

void generateMissingBMP(wxBitmap&, const wxSize& = wxDefaultSize);

void doColor(wxBitmap&, const color::Dynamic&);

} // namespace

Bitmap::Bitmap() = default;

Bitmap::Bitmap(std::string_view src, Type type) {
    // In this function, the load can trigger a log dialog if not found. I use
    // the missing bitmap as a fallback, so the dialog is unwanted.
    wxLogNull noErrors;

    switch (type) {
        case Type::Normal:
            LoadFile(
                (paths::resourceDir() / "icons" /
                (std::string{src} + ".png")).string(),
                wxBITMAP_TYPE_PNG
            );
            break;
#       ifdef __APPLE__
        case Type::Resource:
            LoadFile(
                wxString{src},
                wxBITMAP_TYPE_ICON_RESOURCE
            );
            break;
#       endif
    }

    if (not IsOk()) generateMissingBMP(*this);

    wxDisplay display(0U);
    // TODO: Always fetching the first display is not a great way of doing
    // this, and I don't recall in what situations it's even necessary!
    if (display.IsOk()) SetScaleFactor(display.GetScaleFactor());
}

Bitmap::Bitmap(const wxBitmap& other) : wxBitmap(other) {}

Bitmap::Bitmap(wxBitmap&& other) : wxBitmap(std::move(other)) {}

#ifdef _WIN32
Bitmap::Bitmap(const wxIcon& icon) : wxBitmap(icon) {}
#endif

Bitmap& Bitmap::scale(float64 scale) {
#   ifdef _WIN32
    static_cast<wxBitmap&>(*this) = ConvertToImage().Scale(
        static_cast<int32>(GetWidth() / scale),
        static_cast<int32>(GetHeight() / scale)
    );
#   else
    SetScaleFactor(GetScaleFactor() * scale);
#   endif

    return *this;
}

Bitmap& Bitmap::scaleTo(uint32 dim, wxOrientation orient) {
    float64 scaler{};
    if (orient == wxHORIZONTAL) {
        scaler = GetLogicalWidth() / dim;
    } else {
        scaler = GetLogicalHeight() / dim;
    }

    scale(scaler);

    return *this;
}

Bitmap& Bitmap::pad(
    uint32 padding, int32 withDim, wxOrientation orient
) {
    if (withDim == -1) {
        auto size{GetLogicalSize()};
        withDim = orient == wxHORIZONTAL ? size.GetWidth() : size.GetHeight();
    }

    assert(padding * 2 < withDim);
    const auto adjustedDim{withDim - (padding * 2)};

    float64 retScaler{};
    wxRealPoint offset; 
    if (orient == wxHORIZONTAL) {
        retScaler = static_cast<float64>(withDim) / GetLogicalWidth();

        const auto padScaler{GetLogicalHeight() / GetLogicalWidth()};
        offset = {static_cast<float64>(padding), padScaler * padding};
    } else {
        retScaler = static_cast<float64>(withDim) / GetLogicalHeight();

        const auto padScaler{GetLogicalWidth() / GetLogicalHeight()};
        offset = {padScaler * padding, static_cast<float64>(padding)};
    }
    wxSize retSize{GetLogicalSize() * retScaler};

    scaleTo(adjustedDim, orient);

    wxBitmap padded(retSize, 32);
    padded.UseAlpha();

    wxMemoryDC dc(padded);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();

#   ifdef __WXGTK__
    // On GTK, images seem fairly biased towards drawing in the upper left,
    // particularly the left, so this is a hacky way to try and level that
    // out.
    offset.x += 1;
#   endif

    dc.DrawBitmap(*this, offset);

    dc.SelectObject(wxNullBitmap);

    static_cast<wxBitmap&>(*this) = std::move(padded);

    return *this;
}

Bitmap& Bitmap::color(const color::Dynamic& dyn) {
    mColor = dyn;
    return *this;
}

Bitmap::operator bool() const {
    return IsOk();
}

wxBitmap Bitmap::realize() const {
    wxBitmap ret{static_cast<const wxBitmap&>(*this)};
    doColor(ret, mColor);
    return ret;
}

namespace {

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

void doColor(wxBitmap& bmp, const color::Dynamic& dyn) {
    const auto color{dyn.color()};
    if (not color.IsOk()) return;

    wxAlphaPixelData data(bmp);
    if (not data) return;

    auto iter{data.GetPixels()};
    for (auto yIdx{0}; yIdx < data.GetHeight(); ++yIdx) {
        wxAlphaPixelData::Iterator rowStart = iter;

        for (auto xIdx{0}; xIdx < data.GetWidth(); ++xIdx, ++iter) {
            if (iter.Alpha() > 0) {
#               ifdef _WIN32
                // Idk man windows is funky with its bitmaps.
                auto alphaScale{static_cast<float64>(iter.Alpha()) / 0xFF};
                iter.Red() = static_cast<uint8>(color.Red() * alphaScale);
                iter.Green() = static_cast<uint8>(color.Green() * alphaScale);
                iter.Blue() = static_cast<uint8>(color.Blue() * alphaScale);
#               else
                iter.Red() = color.Red();
                iter.Green() = color.Green();
                iter.Blue() = color.Blue();
#               endif
            }
        }

        iter = rowStart;
        iter.OffsetY(data, 1);
    }
}

} // namespace

