#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/versions/versions.h
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

#include "ui/controls/version.h"
#include "utils/types.h"
#include "utils/version.h"

#include "prop.h"

#include "versions_export.h"

namespace Versions {

constexpr cstring INFO_FILE_STR{"info.pconf"};
constexpr cstring DATA_FILE_STR{"data.pconf"};
constexpr cstring HEADER_FILE_STR{"header.h"};
constexpr cstring DEFAULT_CORE_VERSION{"3.6.0"};

struct VERSIONS_EXPORT VersionedOS {
    VersionedOS();
    Utils::Version verNum;
    Utils::Version coreVersion;
    string coreURL;
    string coreBoardV1;
    string coreBoardV2;
    string coreBoardV3;
};

struct VERSIONS_EXPORT VersionedProp {
    VersionedProp(string name) : name{std::move(name)} {}

    const string name;
    std::unique_ptr<const Prop> prop;

    void addVersion();
    void removeVersion(uint32 idx);

    [[nodiscard]] const vector<std::unique_ptr<PCUI::VersionData>>& supportedVersions() {
        return mSupportedVersions;
    }

private:
    friend VERSIONS_EXPORT void loadLocal();
    void saveInfo();
    vector<std::unique_ptr<PCUI::VersionData>> mSupportedVersions;
};

VERSIONS_EXPORT void loadLocal();

VERSIONS_EXPORT const VersionedOS *getVersionedOS(const Utils::Version&);

/**
 * @return versions sorted from latest to oldest
 */
VERSIONS_EXPORT const vector<VersionedOS>& getOSVersions();

VERSIONS_EXPORT const vector<std::unique_ptr<VersionedProp>>& getProps();

/**
 * @return all registered props for the version
 * Must be copied into config (or elsewhere if you're crazy like that) for use.
 */
VERSIONS_EXPORT vector<VersionedProp *> propsForVersion(const Utils::Version&);

} // namespace Versions
