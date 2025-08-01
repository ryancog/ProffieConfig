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

#include "utils/types.h"
#include "utils/version.h"

#include "prop.h"

#include "private/export.h"

namespace Versions {

struct OSVersion {
    Utils::Version verNum;
    Utils::Version coreVersion;
};

struct PropVersions {
    std::shared_ptr<const Prop> prop;
    vector<Utils::Version> supportedVersions;
};

void VERSIONS_EXPORT loadLocal();

/**
 * @return versions sorted from latest to oldest
 */
const vector<OSVersion>& VERSIONS_EXPORT getOSVersions();

const vector<PropVersions>& VERSIONS_EXPORT getProps();

/**
 * @return all registered props for the version
 * Must be copied into config (or elsewhere if you're crazy like that) for use.
 */
vector<std::shared_ptr<const Prop>> VERSIONS_EXPORT propsForVersion(Utils::Version);

}
