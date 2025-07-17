#include "array.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/preset/array.cpp
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

#include <algorithm>

namespace Config {

} // namespace Config

Config::PresetArray::PresetArray(Config& parent, UID id, string name) : 
    Tracked(id), mParent(parent), mName(std::move(name)) {}

const string& Config::PresetArray::getName() const { return mName; }

bool Config::PresetArray::changeName(const string& name) {
    // Find invalid chars, if any.
    if (std::find_if(name.begin(), name.end(), [](char chr){ 
                if (std::isalpha(chr)) return false;
                if (chr == '_') return false;
                return true;
                }) != name.end()) return false;
    mName = name;
    return true;
}

Config::Preset& Config::PresetArray::addPreset() {
    mPresets.push_back({});
    return mPresets.back();
}

const std::vector<Config::Preset>& Config::PresetArray::getPresets() { return mPresets; }

void Config::PresetArray::syncWithBladeArrays(uint32 numBlades) {
    for (auto& preset : mPresets) {
        preset.styles.resize(numBlades);
    }
}

