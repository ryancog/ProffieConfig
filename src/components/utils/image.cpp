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

#include <iostream>
#include <unordered_map>

#include <wx/bitmap.h>

#include <utils/paths.h>

namespace Image {

std::unordered_map<string, wxBitmap> bmps;

} // namespace Image

wxBitmap Image::loadPNG(const string& name, bool dpiScaled) {
    auto bmpIt{bmps.find(name)};
    if (bmpIt != bmps.end()) return bmpIt->second;

    auto pngPath{Paths::resources() / "icons" / (name + ".png")};
    // std::cout << "Loading PNG \"" << name << "\" from \"" << pngPath.string() << '"' << std::endl;
    auto bitmap{wxBitmap(pngPath.string(), wxBITMAP_TYPE_PNG)};
    if (dpiScaled) bitmap.SetScaleFactor(getDPIScaleFactor());

    bmps.emplace(name, bitmap);
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

