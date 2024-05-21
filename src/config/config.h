#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * config/config.h
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

#include <string>

#include "settings.h"

namespace Config {

struct Data;
typedef std::unordered_map<std::string, Setting::DefineMap*> PropMap;
typedef std::unordered_map<std::string, std::string> CDefineMap;

Data* readConfig(const std::string& filename);
void writeConfig(const std::string& filename, Data& config);

struct Data {
    Setting::Combo<Setting::SettingBase> proffieboard;
    Setting::Combo<Setting::SettingBase> selectedProp;

    Setting::DefineMap generalDefines;
    CDefineMap customDefines;
    PropMap propDefines;

    Setting::Numeric<Setting::SettingBase> maxLedsPerStrip;

    ~Data() {
        for (auto& propDef : propDefines) if (propDef.second) delete propDef.second;
    }
};

}
