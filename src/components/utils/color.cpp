#include "color.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/color.cpp
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

const wxColour color::DARK_BLUE{39, 74, 114};
const wxColour color::LIGHT_BLUE{31, 99, 168};

color::Dynamic::Dynamic() :
    mType{Type::STANDARD}, mStd{.dark_=wxNullColour, .light_=wxNullColour} {}

color::Dynamic::Dynamic(wxColour dark, wxColour light) :
    mType{Type::STANDARD}, mStd{.dark_=std::move(dark), .light_=std::move(light)} {}

color::Dynamic::Dynamic(wxSystemColour color) :
    mType{Type::SYSTEM}, mSysColor{color} {}

color::Dynamic::~Dynamic() {
    if (mType == Type::STANDARD) {
        mStd.dark_.~wxColour();
        mStd.light_.~wxColour();
    }
}

wxColour color::Dynamic::color() const {
    if (mType == Type::SYSTEM) {
        return wxSystemSettings::GetColour(mSysColor);
    }
    if (mType == Type::STANDARD) {
        auto isDark{wxSystemSettings::GetAppearance().AreAppsDark()};

        if (not mStd.dark_.IsOk()) return mStd.light_;
        if (not mStd.light_.IsOk()) return mStd.dark_;

        return isDark ? mStd.light_ : mStd.dark_;
    }
    return wxNullColour;
}

color::Dynamic::Dynamic(const Dynamic& other) {
    mType = Type::STANDARD;
    new(&mStd.light_) wxColour;
    new(&mStd.dark_) wxColour;
    *this = other;
}

color::Dynamic& color::Dynamic::operator=(const Dynamic& other) {
    if (&other == this) return *this;
    
    mType = other.mType;
    switch (mType) {
        case Type::STANDARD:
            mStd.dark_ = other.mStd.dark_;
            mStd.light_ = other.mStd.light_;
            break;
        case Type::SYSTEM:
            mSysColor = other.mSysColor;
            break;
    }

    return *this;
}

color::Dynamic::operator bool() const { 
    switch (mType) {
        case Type::STANDARD: 
            return mStd.dark_.IsOk() or mStd.light_.IsOk();
        case Type::SYSTEM:
            return mSysColor < wxSYS_COLOUR_MAX;
    }

    assert(0);
    __builtin_unreachable();
}

