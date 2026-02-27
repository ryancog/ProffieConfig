#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/versions/versions.hpp
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

#include <utility>
#include <optional>
#include <string>

#include "log/branch.h"
#include "ui/controls/version.h"
#include "utils/string.hpp"
#include "utils/types.hpp"
#include "utils/version.h"

#include "prop.h"

#include "versions_export.h"

namespace versions {

constexpr cstring INFO_FILE_STR{"info.pconf"};
constexpr cstring DATA_FILE_STR{"data.pconf"};
constexpr cstring HEADER_FILE_STR{"header.h"};

constexpr utils::TrimRules PROP_NAME_RULES{
    .allowAlpha = true,
    .allowNum = true,
    .safeList = "_",
};

VERSIONS_EXPORT Utils::Version getDefaultCoreVersion();
VERSIONS_EXPORT Utils::Version getDefaultOSVersion();

struct VERSIONS_EXPORT VersionedOS {
    VersionedOS();
    Utils::Version verNum;
    Utils::Version coreVersion;
    std::string coreURL;
    std::string coreBoardV1;
    std::string coreBoardV2;
    std::string coreBoardV3;
};

struct VERSIONS_EXPORT VersionedProp {
    VersionedProp(std::string name) : name{std::move(name)} {}

    const std::string name;
    std::unique_ptr<const Prop> prop;

    bool addVersion();
    bool removeVersion(uint32 idx);

    using SupportedVersionList = vector<std::unique_ptr<pcui::VersionData>>;

    [[nodiscard]] const SupportedVersionList& supportedVersions() {
        return mSupportedVersions;
    }

private:
    SupportedVersionList mSupportedVersions;
};

VERSIONS_EXPORT std::optional<std::string> resetToDefault(
    bool purge, Log::Branch * = nullptr
);

VERSIONS_EXPORT void loadLocal();

VERSIONS_EXPORT const VersionedOS *getVersionedOS(const Utils::Version&);

/**
 * @return versions sorted from latest to oldest
 */
VERSIONS_EXPORT const std::vector<VersionedOS>& getOSVersions();

VERSIONS_EXPORT const std::vector<std::unique_ptr<VersionedProp>>& getProps();

/**
 * @return all registered props for the version
 * Must be copied into config (or elsewhere if you're crazy like that) for use.
 */
VERSIONS_EXPORT std::vector<VersionedProp *> propsForVersion(const Utils::Version&);

} // namespace versions

