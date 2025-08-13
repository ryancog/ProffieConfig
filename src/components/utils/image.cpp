#include "image.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include <unordered_map>

#include <wx/bitmap.h>
#include <wx/rawbmp.h>
#include <wx/settings.h>

#include "paths/paths.h"
#include "wx/gdicmn.h"

namespace Image {

// std::unordered_map<string, wxBitmap> bmps;
const wxColour DARK_BLUE{39, 74, 114};
const wxColour LIGHT_BLUE{31, 99, 168};

} // namespace Image

Image::DynamicColor::DynamicColor() :
    mType{Type::STANDARD}, mStd{wxNullColour, wxNullColour} {}

Image::DynamicColor::DynamicColor(wxColour dark, wxColour light) :
    mType{Type::STANDARD}, mStd{dark, light} {}

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
}

wxBitmap Image::loadPNG(const string& name, bool dpiScaled) {
    // auto bmpIt{bmps.find(name)};
    // if (bmpIt != bmps.end()) return bmpIt->second;

    auto pngPath{Paths::resources() / "icons" / (name + ".png")};
    // std::cout << "Loading PNG \"" << name << "\" from \"" << pngPath.native() << '"' << std::endl;
    auto bitmap{wxBitmap{pngPath.native(), wxBITMAP_TYPE_PNG}};
    if (dpiScaled) bitmap.SetScaleFactor(getDPIScaleFactor());

    // bmps.emplace(name, bitmap);
    return bitmap;
}

wxBitmap Image::loadPNG(const string& name, wxSize size, wxColour color) {
    auto pngPath{Paths::resources() / "icons" / (name + ".png")};
    auto bitmap{wxBitmap{pngPath.native(), wxBITMAP_TYPE_PNG}};

    if (size.x != -1 or size.y != -1) {
        assert(size.x == -1 or size.y == -1);
        float64 scaler;
        if (size.x != -1) {
            scaler = bitmap.GetLogicalWidth() / size.x;
        } else {
            scaler = bitmap.GetLogicalHeight() / size.y;
        }

        bitmap.SetScaleFactor(bitmap.GetScaleFactor() * scaler);
    }

    if (not color.IsOk()) {
        wxAlphaPixelData data{bitmap};
        auto iter{data.GetPixels()};
        for (auto idx{0}; idx < data.GetWidth() * data.GetHeight(); ++idx) {
            if (iter.Alpha() > 0) {
                iter.Red() = color.Red();
                iter.Green() = color.Green();
                iter.Blue() = color.Blue();
            }
            ++iter;
        }
    }

    return bitmap;
}

wxBitmap Image::newBitmap(wxSize size) {
    wxBitmap bmp;
    bmp.CreateWithDIPSize(size, getDPIScaleFactor(), 32);
    bmp.UseAlpha();
    return std::move(bmp);
}

int32 Image::getDPIScaleFactor() { return 2; }

wxColour Image::getAccentColor() { return { 0x27, 0x4a, 0x72 }; }

