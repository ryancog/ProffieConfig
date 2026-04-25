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
#include "versions/os.hpp"
#include "versions/prop.hpp"

namespace fs = std::filesystem;

namespace config {

constexpr cstring RAW_FILE_EXTENSION{".h"};
constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config : data::Root {
    ~Config() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    std::optional<versions::os::OSData> osVersion() const;

    const data::Vector& osVersions() const;
    const data::Selector osVersion_; 

    const data::Vector *boards() const;
    const data::Selector& boardSel() const;
    const versions::os::Board *board() const;

    const data::Selector& propSel() const;
    const data::Vector *props() const;
    const versions::props::Prop *prop() const;

    Settings settings_;
    data::Vector presetArrays_;
    data::Vector bladeConfigs_;

    data::Vector buttons_;
    data::Vector injections_;

    const data::Bool& isSaved() const;

    const data::Integer& numBlades() const;

    void calcNumBlades();
    void syncStyles() const;

private:
    friend struct Info;

    Config();

    void ensurePropsForVersion();

    data::Vector mOsVersions;
    std::map<
        utils::Version, data::Vector, utils::Version::RawOrderer
    > mPropMap;
    data::Selector mPropSel;
    data::Selector mBoardSel;

    data::Integer mNumBlades;

    data::Bool mIsSaved;

    struct SavedReceiver;
    SavedReceiver *mRcvr;
    std::optional<size> mSavedAction;
};

struct CONFIG_EXPORT Info : data::Model {
    const data::String& name();

    fs::path path();

    [[nodiscard]] std::optional<std::string> save(
        logging::Branch * = nullptr
    );

    std::optional<std::string> load();
    void unload();

    const std::unique_ptr<Config>& config();

private:
    friend void update();

    Info();

    data::String mName;
    std::unique_ptr<Config> mConfig;
};

CONFIG_EXPORT const data::Vector& list();

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

CONFIG_EXPORT std::optional<std::string> generate(
    const Config&, const fs::path&, logging::Branch * = nullptr
);

} // namespace config

