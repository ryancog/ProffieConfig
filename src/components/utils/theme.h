#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/theme.h
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

#include <wx/event.h>
#include <wx/settings.h>
#include <wx/window.h>

#include <utils/types.h>

#include "private/export.h"

namespace Theme {

enum ColorPlane {
    FOREGROUND,
    BACKGROUND,
};

struct UTILS_EXPORT ColorInfo {
    ColorInfo(int8 lightOffset, int8 darkOffset, ColorPlane colorPlane = BACKGROUND, ColorPlane refPlane = BACKGROUND, wxWindow *refWindow = nullptr) :
        lightOffset(lightOffset), darkOffset(darkOffset), colorPlane(colorPlane), referencePlane(refPlane), referenceWindow(refWindow) {}

    int8 lightOffset;
    int8 darkOffset;
    ColorPlane colorPlane;
    ColorPlane referencePlane;
    wxWindow *referenceWindow{nullptr};
};

UTILS_EXPORT void colorWindow(wxWindow *, vector<ColorInfo>);

} // namespace Theme

