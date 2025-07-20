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

#include "versions/prop.h"
#include "preset/array.h"
#include "bladeconfig/arrays.h"
#include "private/export.h"
#include "settings/settings.h"

namespace Config {

static constexpr cstring RAW_FILE_EXTENSION{".h"};

static constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config {
    PCUI::TextData name;
    Settings settings;

    PCUI::ChoiceData propSelection;
    vector<Versions::Prop> props;

    PresetArrays presetArrays;

    BladeArrays bladeArrays;
};

/**
 * Search disk and retrieve list of all config names
 */
vector<string> CONFIG_EXPORT fetchListFromDisk();

void CONFIG_EXPORT rename(const string& oldName, const string& newName);

bool CONFIG_EXPORT remove(const string& name);

/**
 * Parse the config and return a fresh ptr, or return the ptr
 * of an already-open config.
 */
std::shared_ptr<Config> open(const string& name);

bool CONFIG_EXPORT save(std::shared_ptr<Config>, filepath = {});

/**
 * Remove from internal storage and let it die once last memory
 * is forgotten...
 */
bool CONFIG_EXPORT close(std::shared_ptr<Config>);

} // namespace Config

