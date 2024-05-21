#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * stylemanager/stylemanager.h
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

#include <wx/window.h>

#include "styles/bladestyle.h"

namespace StyleManager {

struct Preset {
    std::string name;
    BladeStyles::BladeStyle* style{nullptr};

    ~Preset() {
        if (style) delete style;
    }
};
typedef std::unordered_map<std::string, Preset> StyleMap;

void launch(wxWindow* parent);
void saveStyles(const StyleMap&);
StyleMap* loadStyles();

}
