#include "image.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/image.cpp
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

#include <utility>

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/rawbmp.h>
#include <wx/settings.h>

#include "utils/paths.h"

const wxColour Image::DARK_BLUE{39, 74, 114};
const wxColour Image::LIGHT_BLUE{31, 99, 168};

namespace {

// std::unordered_map<string, wxBitmap> bmps;
void generateMissingBMP(wxBitmap&, const wxSize& = wxDefaultSize);

} // namespace

Image::DynamicColor::DynamicColor() :
    mType{Type::STANDARD}, mStd{.mDark=wxNullColour, .mLight=wxNullColour} {}

Image::DynamicColor::DynamicColor(wxColour dark, wxColour light) :
    mType{Type::STANDARD}, mStd{.mDark=std::move(dark), .mLight=std::move(light)} {}

Image::DynamicColor::DynamicColor(wxSystemColour color) :
    mType{Type::SYSTEM}, mSysColor{color} {}

Image::DynamicColor::~DynamicColor() {
    if (mType == Type::STANDARD) {
        mStd.mDark.~wxColour();
        mStd.mLight.~wxColour();
    }
}

wxColour Image::DynamicColor::color() const {
    if (mType == Type::SYSTEM) {
        return wxSystemSettings::GetColour(mSysColor);
    }
    if (mType == Type::STANDARD) {
        auto isDark{wxSystemSettings::GetAppearance().AreAppsDark()};

        if (not mStd.mDark.IsOk()) return mStd.mLight;
        if (not mStd.mLight.IsOk()) return mStd.mDark;

        return isDark ? mStd.mLight : mStd.mDark;
    }
    return wxNullColour;
}

Image::DynamicColor::DynamicColor(const DynamicColor& other) {
    mType = Type::STANDARD;
    new(&mStd.mLight) wxColour;
    new(&mStd.mDark) wxColour;
    *this = other;
}

Image::DynamicColor& Image::DynamicColor::operator=(const DynamicColor& other) {
    if (&other == this) return *this;
    
    mType = other.mType;
    switch (mType) {
        case Type::STANDARD:
            mStd.mDark = other.mStd.mDark;
            mStd.mLight = other.mStd.mLight;
            break;
        case Type::SYSTEM:
            mSysColor = other.mSysColor;
            break;
    }

    return *this;
}

Image::DynamicColor::operator bool() const { 
    switch (mType) {
        case Type::STANDARD: 
            return mStd.mDark.IsOk() or mStd.mLight.IsOk();
        case Type::SYSTEM:
            return mSysColor < wxSYS_COLOUR_MAX;
    }

    assert(0);
    __builtin_unreachable();
}

wxBitmap Image::loadPNG(const string& name) {
    // auto bmpIt{bmps.find(name)};
    // if (bmpIt != bmps.end()) return bmpIt->second;

    auto pngPath{Paths::resourceDir() / "icons" / (name + ".png")};
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

wxBitmap Image::loadPNG(const string& name, wxSize size, const wxColour& color) {
    auto pngPath{Paths::resourceDir() / "icons" / (name + ".png")};
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

    if (size.x != -1 or size.y != -1) {
        assert(size.x == -1 or size.y == -1);
        float64 scaler{};

        if (size.x != -1) {
            scaler = bitmap.GetLogicalWidth() / size.x;
        } else {
            scaler = bitmap.GetLogicalHeight() / size.y;
        }

#       ifdef _WIN32
        auto img{bitmap.ConvertToImage()};
        bitmap = img.Scale(
            static_cast<int32>(bitmap.GetWidth() / scaler),
            static_cast<int32>(bitmap.GetHeight() / scaler)
        );
#       else
        bitmap.SetScaleFactor(bitmap.GetScaleFactor() * scaler);
#       endif
    }

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

wxBitmap Image::newBitmap(wxSize size) {
    wxBitmap bmp;
    bmp.CreateWithDIPSize(size, wxDisplay{0U}.GetScaleFactor(), 32);
    bmp.UseAlpha();
    return std::move(bmp);
}

wxColour Image::getAccentColor() { return { 0x27, 0x4a, 0x72 }; }

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
    bmpDC.SetBrush(wxBrush(Image::LIGHT_BLUE));
    bmpDC.DrawRoundedRectangle(0, 0, dimension, dimension, 4);

    auto extent{bmpDC.GetTextExtent("?")};
    extent /= 2;
    dimension /= 2;
    bmpDC.SetTextForeground(Image::DARK_BLUE);
    bmpDC.DrawText("?", dimension - extent.x, dimension - extent.y);
}

} // namespace

