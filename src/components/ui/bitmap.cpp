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

wxBitmap generate_missing_bmp(const wxSize& = wxDefaultSize);
void do_color(wxBitmap&, const color::Dynamic&);

} // namespace

Bitmap::Bitmap() = default;

Bitmap::Bitmap(std::string_view src, Type type) {
    // In this function, the load can trigger a log dialog if not found. I use
    // the missing bitmap as a fallback, so the dialog is unwanted.
    wxLogNull no_errors;

    switch (type) {
        case Type::Normal:
            LoadFile(
                (paths::resourceDir() / "icons" /
                (std::string{src} + ".png")).string(),
                wxBITMAP_TYPE_PNG
            );
            break;
#       if __APPLE__
        case Type::Resource:
            LoadFile(
                wxString{src},
                wxBITMAP_TYPE_ICON_RESOURCE
            );
            break;
#       endif
    }

    if (not IsOk())
        return;

    wxDisplay display(0U);
    // TODO: Always fetching the first display is not a great way of doing
    // this, and I don't recall in what situations it's even necessary!
    if (display.IsOk())
        SetScaleFactor(display.GetScaleFactor());
}

Bitmap::Bitmap(const wxBitmap& other) : wxBitmap(other) {}

Bitmap::Bitmap(wxBitmap&& other) : wxBitmap(std::move(other)) {}

#if _WIN32
Bitmap::Bitmap(const wxIcon& icon) : wxBitmap(icon) {}
#endif

Bitmap& Bitmap::scale(float64 scale) {
    if (not IsOk())
        return *this;

    auto size{GetDIPSize()};

    static_cast<wxBitmap&>(*this) = ConvertToImage().Scale(
        static_cast<int32>(size.GetWidth() * scale),
        static_cast<int32>(size.GetHeight() * scale)
    );

    return *this;
}

Bitmap& Bitmap::scale_to(uint32 dim, wxOrientation orient) {
    if (not IsOk())
        return *this;

    auto size{GetDIPSize()};

    float64 scaler{};
    if (orient == wxHORIZONTAL) {
        scaler = static_cast<float64>(dim) / size.GetWidth();
    } else {
        scaler = static_cast<float64>(dim) / size.GetHeight();
    }

    scale(scaler);

    return *this;
}

Bitmap& Bitmap::pad(
    uint32 padding,
    int32 with_dim,
    wxOrientation orient
) {
    if (not IsOk())
        return *this;

    if (with_dim != -1)
        // If dimension is specified, scale this down to fit w/ padding first.
        scale_to(with_dim - (padding * 2), orient);

    auto size{GetDIPSize()};
    size.IncBy(2 * static_cast<int32>(padding));

    wxPoint offset(
        static_cast<int32>(padding),
        static_cast<int32>(padding)
    );

#   if __WXGTK__
    // On GTK, images seem fairly biased towards drawing in the upper left,
    // particularly the left, so this is a hacky way to try and level that
    // out.
    offset.x += 1;
#   endif

    wxBitmap padded(GetScaleFactor() * size, 32);
    padded.UseAlpha();
    padded.SetScaleFactor(GetScaleFactor());

    { wxMemoryDC dc(padded);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        dc.Clear();
        dc.DrawBitmap(*this, offset);
    }

    static_cast<wxBitmap&>(*this) = std::move(padded);
    return *this;
}

Bitmap& Bitmap::color(const color::Dynamic& dyn) {
    m_color = dyn;
    return *this;
}

Bitmap::operator bool() const {
    return IsOk();
}

wxBitmap Bitmap::realize() const {
    if (not IsOk())
        return generate_missing_bmp();

    wxBitmap ret(static_cast<const wxBitmap&>(*this));
    do_color(ret, m_color);

    return ret;
}

namespace {

wxBitmap generate_missing_bmp(const wxSize& size) {
    const int32 dimension{[&] {
        int32 ret{std::max(size.GetWidth(), size.GetHeight())};

        if (ret == wxDefaultCoord)
            return 32;

        return ret;
    }()};

    wxBitmap bmp(dimension, dimension, 32);
    bmp.UseAlpha(true);

    { wxMemoryDC dc(bmp);
        dc.SetFont(wxFont{
            static_cast<int32>(dimension * 0.8),
            wxFONTFAMILY_MODERN,
            wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_BOLD
        });

        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(0, 0, dimension, dimension);

        dc.SetBrush(wxBrush(LIGHT_BLUE));
        dc.DrawRoundedRectangle(0, 0, dimension, dimension, 4);

        const auto extent{dc.GetTextExtent("?")};
        const auto margin{wxSize(dimension, dimension) - extent};
        dc.SetTextForeground(DARK_BLUE);
        dc.DrawText("?", margin.x / 2, margin.y / 2);
    }

    return bmp;
}

void do_color(wxBitmap& bmp, const color::Dynamic& dyn) {
    const auto color{dyn.color()};
    if (not color.IsOk())
        return;

    wxAlphaPixelData data(bmp);
    if (not data)
        return;

    auto iter{data.GetPixels()};
    for (auto y_idx{0}; y_idx < data.GetHeight(); ++y_idx) {
        wxAlphaPixelData::Iterator row_start = iter;

        for (auto x_idx{0}; x_idx < data.GetWidth(); ++x_idx, ++iter) {
            if (iter.Alpha() > 0) {
#               if _WIN32
                // Idk man windows is funky with its bitmaps.
                auto alpha_scale{static_cast<float64>(iter.Alpha()) / 0xFF};
                iter.Red() = static_cast<uint8>(color.Red() * alpha_scale);
                iter.Green() = static_cast<uint8>(color.Green() * alpha_scale);
                iter.Blue() = static_cast<uint8>(color.Blue() * alpha_scale);
#               else
                iter.Red() = color.Red();
                iter.Green() = color.Green();
                iter.Blue() = color.Blue();
#               endif
            }
        }

        iter = row_start;
        iter.OffsetY(data, 1);
    }
}

} // namespace

