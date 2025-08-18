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
#include <fstream>
#include <map>

#include "log/context.h"
#include "pconf/read.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "utils/types.h"

namespace Versions {

constexpr cstring CORE_VERSION_STR{"CORE_VER"};
constexpr cstring SUPPORTED_VERSIONS_STR{"SUPPORTED_VERSIONS"};

const Utils::Version defaultCoreVersion{"3.6.0"};

vector<VersionedOS> osVersions;
vector<std::unique_ptr<VersionedProp>> props;
std::multimap<Utils::Version, VersionedProp *> propVersionMap;

} // namespace Versions

void Versions::loadLocal() {
    auto& logger{Log::Context::getGlobal().createLogger("Versions::loadLocal()")};

    logger.debug("Loading ProffieOS Versions...");
    osVersions.clear();
    for (const auto& entry : fs::directory_iterator(Paths::osDir())) {
        std::error_code err{};
        if (not entry.is_directory(err)) {
            logger.warn("Non-directory OS entry found: " + entry.path().filename().string());
            continue;
        }

        Utils::Version version{entry.path().filename().string()};
        if (version.err) {
            logger.warn("OS dir with invalid version: " + entry.path().filename().string());
            continue;
        }

        bool duplicate{false};
        for (const auto& versionedOS : osVersions) {
            if (versionedOS.verNum == version) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            logger.warn("Duplicate os entry: " + static_cast<string>(version));
            continue;
        }

        logger.debug("Found ProffieOS version " + static_cast<string>(version) + "...");
        VersionedOS os;
        os.verNum = version;

        std::ifstream infoFile{entry.path() / INFO_FILE_STR};
        PConf::Data infoData;
        PConf::read(infoFile, infoData, logger.bverbose("Reading info file..."));
        const auto hashedInfoData{PConf::hash(infoData)};


        const auto coreVersionIter{hashedInfoData.find(CORE_VERSION_STR)};
        if (coreVersionIter != hashedInfoData.end() and coreVersionIter->second->value) {
            Utils::Version coreVersion{*coreVersionIter->second->value};
            if (coreVersion) os.coreVersion = coreVersion;
            else os.coreVersion = defaultCoreVersion;
        } else os.coreVersion = defaultCoreVersion;

        osVersions.push_back(std::move(os));
    }

    logger.debug("Loading Props...");
    props.clear();
    for (const auto& entry : fs::directory_iterator(Paths::propDir())) {
        std::error_code err{};
        if (not entry.is_directory(err)) {
            logger.warn("Non-directory prop entry found: " + entry.path().filename().string());
            continue;
        }

        auto propName{entry.path().filename().string()};
        uint32 numTrimmed{};
        Utils::trimUnsafe(propName, &numTrimmed);
        if (numTrimmed) {
            logger.warn("Prop entry with invalid name: " + entry.path().filename().string());
            continue;
        }

        logger.debug("Found prop " + propName + "...");

        std::ifstream infoFile{Paths::propDir() / propName / INFO_FILE_STR};
        PConf::Data infoData;
        PConf::read(infoFile, infoData, logger.bverbose("Reading info file..."));
        const auto hashedInfoData{PConf::hash(infoData)};

        vector<std::unique_ptr<PCUI::VersionData>> supportedVersions;
        const auto supportedVersionsIter{hashedInfoData.find(SUPPORTED_VERSIONS_STR)};
        if (supportedVersionsIter != hashedInfoData.end()) {
            auto versionStrs{PConf::valueAsList(supportedVersionsIter->second->value)};
            for (const auto& verStr : versionStrs) {
                Utils::Version supportedVersion{verStr};
                if (supportedVersion.err) {
                    logger.warn("Prop " + propName + " lists invalid supported version: " + static_cast<string>(supportedVersion));
                    continue;
                }

                logger.verbose("Prop " + propName + " supports OS version " + static_cast<string>(supportedVersion));
                supportedVersions.push_back(std::make_unique<PCUI::VersionData>(supportedVersion));
            }
        }

        // Yeah this naming is stupid, what are you going to do about it?
        std::ifstream dataFile{Paths::propDir() / propName / DATA_FILE_STR};
        PConf::Data dataData;
        PConf::read(dataFile, dataData, logger.bverbose("Reading data file..."));
        const auto hashedDataData{PConf::hash(dataData)};

        const auto prop{
            Prop::generate(hashedDataData, logger.bverbose("Generating prop..."))
        };
        if (not prop) {
            logger.warn("Failed generating prop " + propName);
            continue;
        }

        auto versionedProp{std::make_unique<VersionedProp>(propName)};
        versionedProp->prop = std::move(prop);
        versionedProp->supportedVersions = std::move(supportedVersions);

        props.push_back(std::move(versionedProp));
    }

    logger.debug("Generating prop version map...");
    propVersionMap.clear();
    for (const auto& prop : props) {
        for (const auto& version : prop->supportedVersions) {
            propVersionMap.emplace(*version, prop.get());
        }
    }

    logger.info("Done");
}

const vector<Versions::VersionedOS>& Versions::getOSVersions() { return osVersions; }
const vector<std::unique_ptr<Versions::VersionedProp>>& Versions::getProps() { return props; }

vector<Versions::VersionedProp *> Versions::propsForVersion(Utils::Version version) {
    vector<VersionedProp *> ret;
    auto [equalIter, equalEnd]{propVersionMap.equal_range(version)};
    for (; equalIter != equalEnd; ++equalIter) {
        ret.push_back(equalIter->second);
    }
    return ret;
}

