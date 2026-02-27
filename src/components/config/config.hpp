#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/config/config.hpp
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

#include <map>

#include "config/blades/array.hpp"
#include "config/presets/array.hpp"
#include "config/settings/settings.hpp"
#include "utils/version.hpp"
// #include "versions/prop.hpp"
// #include "versions/versions.hpp"

#include "config_export.h"

namespace config {

static constexpr cstring RAW_FILE_EXTENSION{".h"};

static constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config {
    data::String name;

    Settings settings;

    data::Vector presetArrays;
    blades::Array bladeArrays;

    [[nodiscard]] bool isSaved() const;

private:
    Config();
};

// /**
//  * Search disk and retrieve list of all config names
//  */
// CONFIG_EXPORT vector<string> fetchListFromDisk();
// 
// /**
//  * @return List of configs currently open
//  */
// CONFIG_EXPORT const vector<std::unique_ptr<Config>>& getOpen();
// 
// CONFIG_EXPORT bool remove(const string& name);
// 
// /**
//  * Parse the config and return a fresh ptr, or return the ptr
//  * of an already-open config.
//  *
//  * @param name Config name
//  *
//  * @return Config or error message
//  */
// CONFIG_EXPORT variant<Config *, string> open(const string& name, Log::Branch * = nullptr);
// 
// /**
//  * Similar to open, but opens from path instead of in save folder by name.
//  *
//  * @return err or nullopt
//  */
// CONFIG_EXPORT optional<string> import(const string& name, const filepath& path);
// 
// /**
//  * @return the config with name only if config is open
//  * nullptr otherwise
//  */
// CONFIG_EXPORT Config *getIfOpen(const string& name);

} // namespace config

