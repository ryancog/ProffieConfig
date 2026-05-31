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
#include <span>

#include "config/settings/settings.hpp"
#include "data/hierarchic/root.hpp"
#include "data/hierarchic/models/vector.hpp"
#include "data/hierarchic/models/choice.hpp"
#include "data/logic/logic.hpp"
#include "data/primitive/model.hpp"
#include "data/primitive/models/bool.hpp"
#include "data/primitive/models/number.hpp"
#include "data/primitive/models/string.hpp"
#include "log/branch.hpp"
#include "utils/data.hpp"
#include "utils/version.hpp"
#include "versions/os.hpp"
#include "versions/prop.hpp"

#include "config_export.h"

namespace fs = std::filesystem;

namespace config {

constexpr cstring RAW_FILE_EXTENSION{".h"};
constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config : data::hier::Root, data::Receiver {
    struct OSIsOrOverVersion {
        utils::Version ver_;
    };

    ~Config() override;

    using Root::children;
    std::vector<const Model *> children() const override;

    std::span<const std::unique_ptr<versions::os::OS>> osVec() const;

    data::hier::Choice& osChoice();
    const data::hier::Choice& osChoice() const;
    const versions::os::OS *os() const;

    data::base::Choice& boardChoice();
    const data::hier::Choice& boardChoice() const;
    const versions::os::Board *board() const;

    std::optional<std::span<const std::unique_ptr<versions::props::Prop>>>
        propVec() const;

    data::hier::Choice& propChoice();
    const data::hier::Choice& propChoice() const;
    const versions::props::Prop *prop() const;

    Settings settings_;
    data::hier::Vector presetArrays_;
    data::hier::Vector bladeConfigs_;

    data::hier::Vector buttons_;
    data::hier::Vector injections_;
    data::hier::Vector styles_;

    const data::prim::Bool& isSaved() const;
    const data::prim::Integer& numBlades() const;

    void calcNumBlades();
    void syncStyles();

    void cache(std::unique_ptr<utils::Data>&&);
    [[nodiscard]] utils::Data *cache() const;

protected:
    std::vector<const Model *> childrenToHash() const override;

private:
    friend struct Info;

    Config();

    void onActivate() override;

    void onAction();

    void onNumBlades();
    void onOSChoice();

    std::vector<const Model *> coreChildren() const;

    static void processPropRecommend(
        data::hier::Root&, std::string_view, std::string_view
    );

    std::vector<std::unique_ptr<versions::os::OS>> mOsVec;
    std::map<
        utils::Version,
        std::vector<std::unique_ptr<versions::props::Prop>>,
        utils::Version::RawOrderer
    > mPropMap;
    data::hier::Choice mOsChoice;
    data::hier::Choice mPropChoice;
    data::hier::Choice mBoardChoice;

    data::prim::Integer mNumBlades;
    data::prim::Bool mIsSaved;

    std::optional<uint64> mSavedHash;
    std::map<uint64, std::unique_ptr<utils::Data>> mCache;
};

CONFIG_EXPORT data::logic::Element operator|(Config&, Config::OSIsOrOverVersion);

struct CONFIG_EXPORT Info : data::prim::Model {
    const data::base::String& name();

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

    data::prim::String mName;
    std::unique_ptr<Config> mConfig;
};

CONFIG_EXPORT const data::base::Vector& list();

/**
 * Search disk and update list of all available configs
 */
CONFIG_EXPORT void update();

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

