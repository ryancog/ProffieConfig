#include "config.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
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

#include "log/context.h"
#include "utils/types.h"
#include "paths/paths.h"
#include "versions/versions.h"

namespace Config {

list<std::shared_ptr<Config>> loadedConfigs;

} // namespace Config

Config::Config::Config() :
    settings{*this},
    bladeArrays{*this} {
    propSelection.setUpdateHandler([this](uint32 id) {
        if (id != propSelection.ID_SELECTION) return;
        propNotifier.notify(ID_PROPSELECTION);
    });

    refreshVersions();
}

void Config::Config::refreshVersions() {
    auto osVersions{Versions::getOSVersions()};
    auto prevOsVer{static_cast<string>(settings.osVersion)};
    int32 newSel{-1};
    vector<string> osChoices;
    osChoices.reserve(osVersions.size());
    for (auto& osVersion : osVersions) {
        auto versionStr{static_cast<string>(osVersion.verNum)};
        if (newSel == -1 and prevOsVer == versionStr) newSel = osChoices.size();
        osChoices.push_back(std::move(osVersion.verNum));
    }

    settings.osVersion.setChoices(std::move(osChoices));
    settings.osVersion = newSel;

    // Show/hide options as necessary.

    refreshPropVersions();
}

void Config::Config::refreshPropVersions() {
    // Blissful ignorance
}

void Config::Config::rename(const string& newName) {
    std::error_code err;
    fs::rename(
        Paths::configs() / (static_cast<string>(name) + RAW_FILE_EXTENSION),
        Paths::configs() / (newName + RAW_FILE_EXTENSION),
        err
    );

    name = string{newName};
}


void Config::Config::close() {
    for (auto iter{loadedConfigs.begin()}; iter != loadedConfigs.end(); ++iter) {
        if (iter->get() == this) {
            loadedConfigs.erase(iter);
            return;
        }
    }
}


bool Config::Config::save(filepath path) {
    if (path.empty()) path = Paths::configs() / (static_cast<string>(name) + RAW_FILE_EXTENSION);
    std::ofstream out{path};
    if (not out.is_open()) return false;

    out << "DUMMY\n";
    out.close();

    return true;
}

bool Config::Config::isSaved() {
    auto& logger{Log::Context::getGlobal().createLogger("EditorWindow::isSaved()")};

    const auto currentPath{
        Paths::configs() / (static_cast<string>(name) + ".h")
    };
    const auto validatePath{
        fs::temp_directory_path() / (static_cast<string>(name) + "-validate")
    };

    auto res{save(validatePath)};

    if (not res) {
        logger.warn("Config output failed");
        return false;
    }

    std::error_code err;
    const auto currentSize{fs::file_size(currentPath, err)};
    const auto validateSize{fs::file_size(validatePath, err)};
    if (currentSize != validateSize) {
        logger.warn(
            "File sizes do not match (" + 
            std::to_string(currentSize) + '/' + 
            std::to_string(validateSize) + ')'
        );
        return false;
    }

    std::ifstream current{currentPath};
    std::ifstream validate{validatePath};

    bool saved{true};
    while (current.good() && !current.eof() && validate.good() && !validate.eof()) {
        std::array<char, 4096> currentBuffer;
        std::array<char, currentBuffer.size()> validateBuffer;
        currentBuffer.fill(0);
        validateBuffer.fill(0);

        current.read(currentBuffer.data(), currentBuffer.size());
        validate.read(validateBuffer.data(), validateBuffer.size());

        if (0 != std::memcmp(currentBuffer.data(), validateBuffer.data(), validateBuffer.size())) {
            saved = false;
            break;
        }
    }

    current.close();
    validate.close();
    fs::remove(validatePath, err);
    if (not saved) {
        logger.warn("File contents do not match");
    }
    return saved;

}

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

vector<std::shared_ptr<Config::Config>> Config::getOpen() {
    vector<std::shared_ptr<Config>> ret;
    ret.assign(loadedConfigs.begin(), loadedConfigs.end());
    return ret;
}

bool Config::remove(const string& name) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) == name) return false;
    }

    std::error_code err;
    return fs::remove(Paths::configs() / (name + RAW_FILE_EXTENSION), err);
}

std::shared_ptr<Config::Config> Config::open(const string& name) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) == name) return config;
    }

    loadedConfigs.push_back(std::shared_ptr<Config>{new Config()});
    auto ret{loadedConfigs.back()};

    if (ret->bladeArrays.arraySelection.choices().empty()) {
        ret->bladeArrays.addArray("blade_in", 0);
    } 
    ret->bladeArrays.arraySelection = 0;

    ret->name = string{name};
    return ret;
}


