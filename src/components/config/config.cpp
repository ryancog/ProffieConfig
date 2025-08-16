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

#include <filesystem>
#include <fstream>

#include "log/context.h"
#include "utils/types.h"
#include "utils/paths.h"
#include "versions/versions.h"

#include "private/io.h"

namespace Config {

vector<std::unique_ptr<Versions::Prop>> Config::mEmptyProps;
vector<std::unique_ptr<Config>> loadedConfigs;

filepath savePath(const string&);

} // namespace Config

Config::Config::Config() :
    settings{*this},
    bladeArrays{*this},
    presetArrays{*this} {
    propSelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_STRING);

    propSelection.setUpdateHandler([this](uint32 id) {
        if (id != propSelection.ID_SELECTION) return;
        propNotifyData.notify(ID_PROPSELECTION);
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
    const auto verNum{settings.getOSVersion()};
    auto& propList{mProps[verNum]};
    for (const auto& versionedProp : Versions::propsForVersion(verNum)) {
        bool found{false};
        for (auto& prop : propList) {
            // Compare by display name, not unique name, obviously
            if (prop->name == versionedProp->prop->name) {
                found = true;
                auto newProp{std::make_unique<Versions::Prop>(*versionedProp->prop)};
                newProp->migrateFrom(*prop);
                prop = std::move(newProp);
                break;
            }
        }

        if (found) continue;

        propList.push_back(std::make_unique<Versions::Prop>(*versionedProp->prop));
    }

    const auto comp{[](
        const std::unique_ptr<Versions::Prop>& a,
        const std::unique_ptr<Versions::Prop>& b
    ) -> bool {
        return a->name < b->name;
    }};
    std::sort(propList.begin(), propList.end(), comp);

    vector<string> choices;
    choices.reserve(propList.size());
    for (const auto& prop : propList) choices.push_back(prop->name);
    propSelection.setChoices(std::move(choices));
}

void Config::Config::rename(const string& newName) {
    std::error_code err;
    fs::rename(savePath(), ::Config::savePath(newName), err);

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

filepath Config::savePath(const string& name) {
    return Paths::configDir() / (static_cast<string>(name) + RAW_FILE_EXTENSION);
}

filepath Config::Config::savePath() const {
    return ::Config::savePath(name);
}

optional<string> Config::Config::save(const filepath& path, Log::Branch *lBranch) const {
    auto& logger{Log::Branch::optCreateLogger("Config::Config::save()", lBranch)};
    logger.info("Saving \"" + static_cast<string>(name) + "\"...");

    optional<string> err;
    std::error_code errCode;
    if (not path.empty()) {
        err = output(path, *this);
        if (err) fs::remove(path, errCode);
        return err;
    } 

    const auto finalPath{savePath()};
    const filepath tmpPath{finalPath.string() + ".tmp"};
    err = output(tmpPath, *this, logger.binfo("Generating output..."));
    if (err) {
        fs::remove(tmpPath, errCode);
        return err;
    }

    if (not fs::copy_file(tmpPath, finalPath, fs::copy_options::overwrite_existing, errCode)) {
        err = errorMessage(logger, wxTRANSLATE("Failed to move temp file: %s"), errCode.message());
    }
    fs::remove(tmpPath, errCode);
    return err;
}

bool Config::Config::isSaved() {
    auto& logger{Log::Context::getGlobal().createLogger("EditorWindow::isSaved()")};

    const auto currentPath{savePath()};
    const auto validatePath{fs::temp_directory_path() / (static_cast<string>(name) + "-validate")};

    auto saveErr{save(validatePath)};

    if (saveErr) {
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
    for (const auto& entry : fs::directory_iterator{Paths::configDir(), err}) {
        if (not entry.is_regular_file()) continue;
        if (entry.path().extension() != RAW_FILE_EXTENSION) continue;
        ret.emplace_back(entry.path().stem().string());
    }
    
    return ret;
}

const vector<std::unique_ptr<Config::Config>>& Config::getOpen() {
    return loadedConfigs;
}

bool Config::remove(const string& name) {
    if (getIfOpen(name)) return false;

    std::error_code err;
    return fs::remove(savePath(name), err);
}

variant<Config::Config *, string> Config::open(const string& name) {
    auto *open{getIfOpen(name)};
    if (open) return open;

    std::unique_ptr<Config> config{new Config()};
    config->name = string{name};

    const auto path{savePath(name)};
    if (fs::exists(path)) {
        auto err{parse(path, *config)};
        if (err) return *err;
    }

    return &*loadedConfigs.emplace_back(std::move(config));
}

optional<string> Config::import(const string& name, const filepath& path) {
    auto& logger{Log::Context::getGlobal().createLogger("Config::import()")};
    for (const auto& configName : fetchListFromDisk()) {
        if (configName == name) {
            return errorMessage(logger, wxTRANSLATE("Config with name already open"));
        }
    }

    std::unique_ptr<Config> config{new Config()};
    config->name = string{name};

    auto err{parse(path, *config, logger.binfo("Parsing config..."))};
    if (err) return err;

    err = config->save();
    if (err) return err;
    
    return nullopt;
}

Config::Config *Config::getIfOpen(const string& name) {
    for (auto& config : loadedConfigs) {
        if (static_cast<string>(config->name) == name) return &*config;
    }
    return nullptr;
}

