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

#include "paths/paths.h"
#include "utils/types.h"

namespace Versions {

inline filepath folder() { return Paths::data() / "versions"; }

vector<OSVersion> osVersions;
vector<PropVersions> props;
std::multimap<Utils::Version, std::shared_ptr<Prop>> propVersionMap;

} // namespace Versions

void Versions::loadLocal() {
    std::error_code err;
    fs::create_directories(folder(), err);

    // TODO: Load
}

const vector<Versions::OSVersion>& Versions::getOSVersions() { return osVersions; }
const vector<Versions::PropVersions>& Versions::getProps() { return props; }

vector<std::shared_ptr<const Versions::Prop>> Versions::propsForVersion(Utils::Version version) {
    vector<std::shared_ptr<const Prop>> ret;
    auto [equalIter, equalEnd]{propVersionMap.equal_range(version)};
    for (; equalIter != equalEnd; ++equalIter) {
        ret.push_back(equalIter->second);
    }
    return ret;
}

