#include "versions.h"
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

#include <filesystem>
#include <map>

#include "utils/paths.h"
#include "utils/types.h"

namespace Versions {

inline filepath folder() { return Paths::dataDir() / "versions"; }

vector<VersionedOS> osVersions;
vector<VersionedProp> props;
std::multimap<Utils::Version, VersionedProp *> propVersionMap;

} // namespace Versions

void Versions::loadLocal() {
    std::error_code err;
    fs::create_directories(folder(), err);

    // TODO: Load
}

const vector<Versions::VersionedOS>& Versions::getOSVersions() { return osVersions; }
const vector<Versions::VersionedProp>& Versions::getProps() { return props; }

vector<Versions::VersionedProp *> Versions::propsForVersion(Utils::Version version) {
    vector<VersionedProp *> ret;
    auto [equalIter, equalEnd]{propVersionMap.equal_range(version)};
    for (; equalIter != equalEnd; ++equalIter) {
        ret.push_back(equalIter->second);
    }
    return ret;
}

