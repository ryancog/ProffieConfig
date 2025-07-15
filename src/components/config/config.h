#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/config.h
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

#include <wx/datetime.h>

#include "preset/preset.h"
#include "private/export.h"
#include "settings/settings.h"
#include "types.h"
#include "wiring/wiring.h"

namespace Config {

static constexpr cstring BUNDLE_FILE_EXTENSION{".prfcfgbundle"};
static constexpr cstring RAW_FILE_EXTENSION{".h"};

static constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config : Tracked {
    wxDateTime modDate;

    string name;
    Wiring::Wiring wiring;
    Settings settings;

    // Returns `PresetArray` if successfully added, nullptr otherwise.
    // If name is empty, a new generic name will be created.
    std::shared_ptr<PresetArray> addPresetArray();
    // True if removed, false otherwise
    bool removePresetArray(UID id);

    std::shared_ptr<PresetArray> getPresetArrayByName(const string& name);

    std::shared_ptr<const PresetArray> getPresetArray(UID id) const;
    std::shared_ptr<PresetArray> getPresetArray(UID id);

    const Map<PresetArray>& getPresetArrays();

private:
    friend std::shared_ptr<Config> addConfig();
    Config(UID);

    Map<PresetArray> mPresetArrays;
};

CONFIG_EXPORT void loadConfigs();
CONFIG_EXPORT void saveConfigs();

CONFIG_EXPORT const Map<Config>& getConfigs();
CONFIG_EXPORT std::shared_ptr<Config> getConfig(UID);

CONFIG_EXPORT std::shared_ptr<Config> addConfig();
// Return true if removed, false if doesn't exist
CONFIG_EXPORT bool removeConfig(UID);

// Doesn't actually reset things, so this assumes the config in the "new"
// state. Trying to use this with an already-used config will end poorly. (probably)
CONFIG_EXPORT void fillWithDefaults(const std::shared_ptr<Config>&);

} // namespace Config

