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

#include <fstream>

#include "utils/types.h"
#include "paths/paths.h"

namespace Config {

set<std::shared_ptr<Config>> loadedConfigs;

} // namespace Config

vector<string> Config::fetchListFromDisk() {
    vector<string> ret;

    std::error_code err;
    for (const auto& entry : fs::directory_iterator{Paths::configs(), err}) {
        if (not entry.is_regular_file()) continue;
        if (entry.path().extension() != RAW_FILE_EXTENSION) continue;
        ret.emplace_back(entry.path().stem());
    }
    
    return ret;
}

void Config::rename(const string& oldName, const string& newName) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) != oldName) continue;

        config->name = string{newName};
    }

    std::error_code err;
    fs::rename(
        Paths::configs() / (oldName + RAW_FILE_EXTENSION),
        Paths::configs() / (newName + RAW_FILE_EXTENSION),
        err
    );
}

bool Config::remove(const string& name) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) == name) return false;
    }

    std::error_code err;
    return fs::remove(Paths::configs() / (name + RAW_FILE_EXTENSION), err);
}

bool Config::close(std::shared_ptr<Config> config) {
    return loadedConfigs.erase(config);
}

std::shared_ptr<Config::Config> Config::open(const string& name) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) == name) return config;
    }

    auto ret{*loadedConfigs.emplace().first};
    ret->name = string{name};
    return ret;
}

bool Config::save(std::shared_ptr<Config> config) {
    std::ofstream out{Paths::configs() / (static_cast<string>(config->name) + RAW_FILE_EXTENSION)};
    if (not out.is_open()) return false;

    out << "DUMMY\n";
    out.close();

    return true;
}

