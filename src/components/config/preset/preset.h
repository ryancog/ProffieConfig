#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/preset/preset.h
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

// #include "styles/bladestyle.h"
#include "utils/types.h"

#include "../private/export.h"
#include "../wiring/wiring.h"

namespace Config {

struct CONFIG_EXPORT Preset {
    string name;
    vector<string> fontDirs;
    string track;
    struct Style {
        string comment;
        string style{"StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()"};
    };
    vector<Style> styles;
    // vector<UID> styles;
};

struct CONFIG_EXPORT PresetArray : Tracked {
    [[nodiscard]] const string& getName() const;
    bool changeName(const string& name);

    Preset& addPreset();
    const std::vector<Preset>& getPresets();

    /**
     * Update the number of styles per preset.
     */
    void syncWithBladeArrays(uint32 numStyles);

private:
    friend class Config;
    PresetArray(Config& parent, UID, string name);

    string mName;
    vector<Preset> mPresets;

    Config& mParent;
};

} // namespace Config

