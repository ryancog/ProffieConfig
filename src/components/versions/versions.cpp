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
#include "pconf/write.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "utils/types.h"

namespace Versions {

constexpr cstring CORE_VERSION_STR{"CORE_VER"};
constexpr cstring CORE_URL_STR{"CORE_URL"};
constexpr cstring CORE_BOARDV1_STR{"CORE_BOARDV1"};
constexpr cstring CORE_BOARDV2_STR{"CORE_BOARDV2"};
constexpr cstring CORE_BOARDV3_STR{"CORE_BOARDV3"};

constexpr cstring SUPPORTED_VERSIONS_STR{"SUPPORTED_VERSIONS"};

const Utils::Version defaultCoreVersion{"3.6.0"};

vector<VersionedOS> osVersions;
vector<std::unique_ptr<VersionedProp>> props;
std::multimap<Utils::Version, VersionedProp *> propVersionMap;

} // namespace Versions

Versions::VersionedOS::VersionedOS() : coreVersion{Utils::Version::invalidObject()} {}

void Versions::VersionedProp::addVersion() {
    auto version{std::make_unique<PCUI::VersionData>()};
    version->setUpdateHandler([this](uint32 id) {
        if (id != PCUI::VersionData::ID_VALUE) return;

        saveInfo();
    });
    mSupportedVersions.push_back(std::move(version));
    saveInfo();
}

void Versions::VersionedProp::removeVersion(uint32 idx) {
    mSupportedVersions.erase(std::next(mSupportedVersions.begin(), idx));
    saveInfo();
}

void Versions::VersionedProp::saveInfo() {
    PConf::Data infoData;

    vector<string> list;
    list.reserve(mSupportedVersions.size());
    for (const auto& version : mSupportedVersions) {
        list.push_back(static_cast<string>(static_cast<Utils::Version>(*version)));
    }
    infoData.push_back(std::make_shared<PConf::Entry>(SUPPORTED_VERSIONS_STR, PConf::listAsValue(list)));

    std::ofstream infoFile{Paths::propDir() / name / INFO_FILE_STR};
    if (not infoFile.is_open()) {
        throw std::ios_base::failure("Couldn't open prop info file for write.");
    }
    PConf::write(infoFile, infoData, nullptr);
}

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
            os.coreVersion = static_cast<Utils::Version>(*coreVersionIter->second->value);
        } 

        const auto coreURLIter{hashedInfoData.find(CORE_URL_STR)};
        if (coreURLIter != hashedInfoData.end() and coreURLIter->second->value) {
            os.coreURL = *coreVersionIter->second->value;
        } 

        const auto coreBoardV1Iter{hashedInfoData.find(CORE_BOARDV1_STR)};
        if (coreBoardV1Iter != hashedInfoData.end() and coreBoardV1Iter->second->value) {
            os.coreBoardV1 = *coreVersionIter->second->value;
        }

        const auto coreBoardV2Iter{hashedInfoData.find(CORE_BOARDV2_STR)};
        if (coreBoardV2Iter != hashedInfoData.end() and coreBoardV2Iter->second->value) {
            os.coreBoardV2= *coreBoardV2Iter->second->value;
        }

        const auto coreBoardV3Iter{hashedInfoData.find(CORE_BOARDV3_STR)};
        if (coreBoardV3Iter != hashedInfoData.end() and coreBoardV3Iter->second->value) {
            os.coreBoardV3 = *coreVersionIter->second->value;
        }

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

        auto versionedProp{std::make_unique<VersionedProp>(propName)};
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
                auto version{std::make_unique<PCUI::VersionData>(supportedVersion)};
                auto *versionedPropPtr{versionedProp.get()};
                version->setUpdateHandler([versionedPropPtr](uint32 id) {
                    if (id != PCUI::VersionData::ID_VALUE) return;

                    versionedPropPtr->saveInfo();
                });
                supportedVersions.push_back(std::move(version));
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

        versionedProp->prop = std::move(prop);
        versionedProp->mSupportedVersions = std::move(supportedVersions);

        props.push_back(std::move(versionedProp));
    }

    logger.debug("Generating prop version map...");
    propVersionMap.clear();
    for (const auto& prop : props) {
        for (const auto& version : prop->mSupportedVersions) {
            propVersionMap.emplace(*version, prop.get());
        }
    }

    logger.info("Done");
}

const Versions::VersionedOS *Versions::getVersionedOS(const Utils::Version& version) {
    for (const auto& versionedOS : osVersions) {
        if (versionedOS.verNum == version) return &versionedOS;
    }

    return nullptr;
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

