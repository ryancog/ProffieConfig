#include "config.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/config.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <memory>
#include <unordered_map>

#include <utils/crypto.h>

#include "config/types.h"
#include "preset/preset.h"

namespace Config {

Map<Config> configs;

} // namespace Config

Config::Config::Config(UID id) : Tracked(id) {}

std::shared_ptr<Config::PresetArray> Config::Config::addPresetArray() {
    auto arrayID{Crypto::genUID(mPresetArrays)};

    auto nameIdx{0};
    constexpr cstring DEFAULT_PRESET_ARRAY_NAME{"presets"};
    string arrayName;
    while (not false) {
        arrayName = DEFAULT_PRESET_ARRAY_NAME;
        if (nameIdx) arrayName += std::to_string(nameIdx);
        if (std::find_if(mPresetArrays.begin(), mPresetArrays.end(), [&arrayName](const Map<PresetArray>::value_type& array) -> bool {
            return array.second->getName() == arrayName;
        }) == mPresetArrays.end()) break;
        nameIdx++;
    }

    auto newArray{std::shared_ptr<PresetArray>(new PresetArray(*this, arrayID, arrayName))};
    mPresetArrays.emplace(arrayID, newArray);
    return newArray;
}

bool Config::Config::removePresetArray(UID id) {
    auto arrayIt{mPresetArrays.find(id)};
    if (arrayIt == mPresetArrays.end()) return false;

    mPresetArrays.erase(arrayIt);
    return true;
}

std::shared_ptr<Config::PresetArray> Config::Config::getPresetArrayByName(const string& name) {
    for (const auto& [ id, array ] : mPresetArrays) {
        if (array->getName() == name) return array;
    }

    return nullptr;
}

std::shared_ptr<const Config::PresetArray> Config::Config::getPresetArray(UID id) const {
    auto arrayIt{mPresetArrays.find(id)};
    if (arrayIt == mPresetArrays.end()) return nullptr;
    return arrayIt->second;
}

std::shared_ptr<Config::PresetArray> Config::Config::getPresetArray(UID id) {
    auto arrayIt{mPresetArrays.find(id)};
    if (arrayIt == mPresetArrays.end()) return nullptr;
    return arrayIt->second;
}

const Config::Map<Config::PresetArray>& Config::Config::getPresetArrays() { return mPresetArrays; }

void Config::loadConfigs() {

}

void Config::saveConfigs() {

}

const std::unordered_map<uint64, std::shared_ptr<Config::Config>>& Config::getConfigs() { return configs; }

std::shared_ptr<Config::Config> Config::getConfig(UID configID) {
    auto configIt{configs.find(configID)};
    if (configIt == configs.end()) return nullptr;
    return configIt->second;
}

std::shared_ptr<Config::Config> Config::addConfig() {
    auto uid{Crypto::genUID(configs)};
    auto newConfig{std::shared_ptr<Config>(new Config(uid))};
    newConfig->modDate.SetToCurrent();
    configs.emplace(uid, newConfig);
    return newConfig;
}

bool Config::removeConfig(UID id) {
    auto configIt{configs.find(id)};
    if (configIt == configs.end()) return false;
    configs.erase(configIt);
    return true;
}

void Config::fillWithDefaults(const std::shared_ptr<Config>& config) {
    constexpr cstring NEW_CONFIG_NAME{"New Config"};
    config->name = NEW_CONFIG_NAME;
    // TODO: See how this changed and if setup for the new blade array system is needed.
    // config->wiring.addBladeArray(0, "Blade In");
}

