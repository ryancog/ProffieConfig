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

#include <filesystem>
#include <map>
#include <memory>

#include "config/settings/settings.hpp"
#include "data/hierarchy/root.hpp"
#include "data/selector.hpp"
#include "data/vector.hpp"

#include "config_export.h"
#include "log/branch.hpp"
#include "utils/version.hpp"

namespace fs = std::filesystem;

namespace config {

constexpr cstring RAW_FILE_EXTENSION{".h"};
constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config : data::Root {
    ~Config() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    const data::Vector& osVersions();
    const data::Selector osVersion_; 

    const data::Vector *boards();
    const data::Selector& board();

    const data::Vector *props();
    const data::Selector& propSel();

    Settings settings_;
    data::Vector presetArrays_;
    data::Vector bladeConfigs_;

    data::Vector buttons_;
    data::Vector injections_;

    const data::Bool& isSaved();

    size numBlades();
    void syncStyles();

private:
    friend struct Info;

    // NOLINTNEXTLINE(modernize-use-equals-delete)
    Config();

    void ensurePropsForVersion();

    data::Vector mOsVersions;
    std::map<
        utils::Version, data::Vector, utils::Version::RawOrderer
    > mPropMap;
    data::Selector mPropSel;
    data::Selector mBoard;
    data::Bool mIsSaved;

    struct SavedReceiver;
    SavedReceiver *mRcvr;
    std::optional<size> mSavedAction;
};

struct CONFIG_EXPORT Info : data::Model {
    // TODO: Same issue as isSaved_
    data::String name_;

    fs::path path();

    [[nodiscard]] std::optional<std::string> save(
        const fs::path& = {}, logging::Branch * = nullptr
    );

    std::optional<std::string> load();
    void unload();

    const std::unique_ptr<Config>& config();

private:
    friend void update();

    Info();

    std::unique_ptr<Config> mConfig;
};

// Another point where an observable is required...
CONFIG_EXPORT extern data::Vector list;

/**
 * Search disk and update list of all available configs
 */
CONFIG_EXPORT void update();

CONFIG_EXPORT bool remove(Info&);

/**
 * Similar to open, but opens from path instead of in save folder by name.
 *
 * @return err or nullopt
 */
CONFIG_EXPORT std::optional<std::string> import(
    const std::string& name, const fs::path& path
);

} // namespace config

