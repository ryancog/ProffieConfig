#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/image.h
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

#include <utils/types.h>

#include "private/export.h"

namespace Image {

UTILS_EXPORT wxBitmap loadPNG(const string& name, bool dpiScaled = true);
UTILS_EXPORT wxBitmap newBitmap(wxSize);
UTILS_EXPORT int32 getDPIScaleFactor();
UTILS_EXPORT wxColour getAccentColor();

} // namespace Image

