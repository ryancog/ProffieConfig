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

#include "ui/controls/text.h"
#include "utils/types.h"
#include "utils/version.h"

#include "prop.h"

#include "private/export.h"

namespace Versions {

struct VersionedOS {
    Utils::Version verNum;
    Utils::Version coreVersion;
};

struct VersionedProp {
    std::shared_ptr<const Prop> prop;
    PCUI::TextData name;
    vector<Utils::Version> supportedVersions;

    /**
     * Move the prop to the location specified by name
     *
     * @return err
     */
    optional<string> syncName();

    /**
     * @return if the name is synced with disk
     */
    bool isSynced() const { return mNameIsSynced; }

private:
    bool mNameIsSynced{true};
};

void VERSIONS_EXPORT loadLocal();

/**
 * @return versions sorted from latest to oldest
 */
const vector<VersionedOS>& VERSIONS_EXPORT getOSVersions();

const vector<VersionedProp>& VERSIONS_EXPORT getProps();

/**
 * @return all registered props for the version
 * Must be copied into config (or elsewhere if you're crazy like that) for use.
 */
vector<VersionedProp *> VERSIONS_EXPORT propsForVersion(Utils::Version);

}
