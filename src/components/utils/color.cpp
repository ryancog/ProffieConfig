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

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/rawbmp.h>
#include <wx/settings.h>

#if __APPLE__
#include <CoreFoundation/CoreFoundation.h>

#include "utils/objc.hpp"
#elif _WIN32
#include <windows.h>
#endif

#include "utils/types.hpp"

namespace {

#if __WXOSX__
wxColour getNSColor(cstring name) {
    static Class NSColorClass{objc_getClass("NSColor")};
    NSColor *color{objcMessage<NSColor *>((id)NSColorClass, name)};
    // Needs explicit ctor
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return wxColour(color);
}
#endif

} // namespace

const wxColour color::DARK_BLUE{39, 74, 114};
const wxColour color::LIGHT_BLUE{31, 99, 168};

color::Dynamic::Dynamic() :
    mType{Type::Standard}, mStd{.dark_=wxNullColour, .light_=wxNullColour} {}

// Passing by value and move doesn't work because of the aggregate?
color::Dynamic::Dynamic(const wxColour& dark, const wxColour& light) :
    mType{Type::Standard}, mStd{.dark_=dark, .light_=light} {}

color::Dynamic::Dynamic(wxSystemColour color) :
    mType{Type::System}, mSysColor{color} {}

color::Dynamic::Dynamic(UserAccent) :
    mType{Type::Accent} {}

color::Dynamic::Dynamic(Special special) :
    mType{Type::Special}, mSpecial{special} {}

color::Dynamic::~Dynamic() {
    if (mType == Type::Standard) {
        mStd.dark_.~wxColour();
        mStd.light_.~wxColour();
    }
}

wxColour color::Dynamic::color() const {
    if (mType == Type::System) {
        return wxSystemSettings::GetColour(mSysColor);
    }

    if (mType == Type::Standard) {
        auto isDark{wxSystemSettings::GetAppearance().AreAppsDark()};

        if (not mStd.dark_.IsOk()) return mStd.light_;
        if (not mStd.light_.IsOk()) return mStd.dark_;

        return isDark ? mStd.light_ : mStd.dark_;
    }

    if (mType == Type::Accent) {
#       if __WXOSX__
        return getNSColor("controlAccentColor");
#       elif __WXMSW__
        DWORD buffer{};
        DWORD bufferSize{sizeof(buffer)};
        auto res{RegGetValueA(
            HKEY_CURRENT_USER,
            R"(Software\Microsoft\Windows\DWM)",
            "AccentColor",
            RRF_RT_REG_DWORD,
            nullptr,
            &buffer,
            &bufferSize
        )};
        if (res != ERROR_SUCCESS)
            return wxNullColour;

        auto r{static_cast<uint8>((buffer >> 0) & 0xFF)};
        auto g{static_cast<uint8>((buffer >> 8) & 0xFF)};
        auto b{static_cast<uint8>((buffer >> 16) & 0xFF)};
        auto a{static_cast<uint8>((buffer >> 24) & 0xFF)};
        return {r, g, b, a};
#       endif
    }

    if (mType == Type::Special) {
        switch (mSpecial) {
            case Special::Caption:
#               if __WXOSX__
                return getNSColor("secondaryLabelColor");
#               else
                return wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
#               endif
                break;
        }
    }

    return wxNullColour;
}

color::Dynamic::Dynamic(const Dynamic& other) {
    mType = Type::Standard;
    new(&mStd.light_) wxColour;
    new(&mStd.dark_) wxColour;
    *this = other;
}

color::Dynamic& color::Dynamic::operator=(const Dynamic& other) {
    if (&other == this) return *this;
    
    mType = other.mType;
    switch (mType) {
        case Type::Standard:
            mStd.dark_ = other.mStd.dark_;
            mStd.light_ = other.mStd.light_;
            break;
        case Type::System:
            mSysColor = other.mSysColor;
            break;
        case Type::Accent:
            mUserAccent = other.mUserAccent;
        case Type::Special:
            mSpecial = other.mSpecial;
            break;
    }

    return *this;
}

color::Dynamic::operator bool() const { 
    switch (mType) {
        case Type::Standard: 
            return mStd.dark_.IsOk() or mStd.light_.IsOk();
        case Type::System:
            return mSysColor < wxSYS_COLOUR_MAX;
        case Type::Accent:
        case Type::Special:
            return true;
    }

    assert(0);
    __builtin_unreachable();
}

