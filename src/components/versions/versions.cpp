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
#include <sstream>

#include <wx/uri.h>
#include <wx/webrequest.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "log/context.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "utils/types.h"

namespace {

constexpr cstring CORE_VERSION_STR{"CORE_VER"};
constexpr cstring CORE_URL_STR{"CORE_URL"};
constexpr cstring CORE_BOARDV1_STR{"CORE_BOARDV1"};
constexpr cstring CORE_BOARDV2_STR{"CORE_BOARDV2"};
constexpr cstring CORE_BOARDV3_STR{"CORE_BOARDV3"};

constexpr cstring SUPPORTED_VERSIONS_STR{"SUPPORTED_VERSIONS"};

vector<Versions::VersionedOS> osVersions;
vector<std::unique_ptr<Versions::VersionedProp>> props;
std::multimap<Utils::Version, Versions::VersionedProp *> propVersionMap;
std::map<Utils::Version, Versions::VersionedProp> propDefaultVersionMap;

} // namespace

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
    infoData.push_back(PConf::Entry::create(SUPPORTED_VERSIONS_STR, PConf::listAsValue(list)));

    auto infoFile{Paths::openOutputFile(Paths::propDir() / name / INFO_FILE_STR)};
    if (not infoFile.is_open()) {
        throw std::ios_base::failure("Couldn't open prop info file for write.");
    }
    PConf::write(infoFile, infoData, nullptr);
}

void Versions::loadLocal() {
    auto& logger{Log::Context::getGlobal().createLogger("Versions::loadLocal()")};

    logger.info("Loading ProffieOS Versions...");
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

        logger.info("Found ProffieOS version " + static_cast<string>(version) + "...");
        VersionedOS os;
        os.verNum = version;

        auto infoFile{Paths::openInputFile(entry.path() / INFO_FILE_STR)};
        PConf::Data infoData;
        PConf::read(infoFile, infoData, logger.bverbose("Reading info file..."));
        const auto hashedInfoData{PConf::hash(infoData)};

        const auto coreVersionEntry{hashedInfoData.find(CORE_VERSION_STR)};
        if (coreVersionEntry and coreVersionEntry->value) {
            os.coreVersion = static_cast<Utils::Version>(*coreVersionEntry->value);
        } 

        const auto coreURLEntry{hashedInfoData.find(CORE_URL_STR)};
        if (coreURLEntry and coreURLEntry->value) {
            os.coreURL = *coreVersionEntry->value;
        } 

        const auto coreBoardV1Entry{hashedInfoData.find(CORE_BOARDV1_STR)};
        if (coreBoardV1Entry and coreBoardV1Entry->value) {
            os.coreBoardV1 = *coreVersionEntry->value;
        }

        const auto coreBoardV2Entry{hashedInfoData.find(CORE_BOARDV2_STR)};
        if (coreBoardV2Entry and coreBoardV2Entry->value) {
            os.coreBoardV2 = *coreVersionEntry->value;
        }

        const auto coreBoardV3Entry{hashedInfoData.find(CORE_BOARDV3_STR)};
        if (coreBoardV3Entry and coreBoardV3Entry->value) {
            os.coreBoardV3 = *coreVersionEntry->value;
        }

        filepath defaultPropDataPath{entry.path() / "default_prop.pconf"};
        auto& versionedProp{propDefaultVersionMap.emplace(version, "").first->second};
        PConf::HashedData hashedDefaultPropData;
        if (fs::is_regular_file(defaultPropDataPath, err)) {
            auto defaultPropDataFile{Paths::openInputFile(defaultPropDataPath)};
            PConf::Data defaultPropData;
            PConf::read(defaultPropDataFile, defaultPropData, logger.bverbose("Reading default prop file..."));
            hashedDefaultPropData = PConf::hash(defaultPropData);
        }
        versionedProp.prop = std::move(Prop::generate(
            hashedDefaultPropData,
            logger.bverbose("Generating default prop...."),
            true
        ));

        osVersions.push_back(std::move(os));
    }

    logger.info("Loading Props...");
    // Migrated or should be removed
    const auto oldProps{std::move(props)};
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

        logger.info("Found prop " + propName + "...");

        VersionedProp *oldPropPtr{};
        for (const auto& oldProp : oldProps) {
            if (oldProp->name == propName) oldPropPtr = oldProp.get();
        }

        auto infoFile{Paths::openInputFile(Paths::propDir() / propName / INFO_FILE_STR)};
        PConf::Data infoData;
        PConf::read(infoFile, infoData, logger.bverbose("Reading info file..."));
        const auto hashedInfoData{PConf::hash(infoData)};

        auto versionedProp{std::make_unique<VersionedProp>(propName)};

        vector<std::unique_ptr<PCUI::VersionData>> supportedVersions;
        const auto supportedVersionsEntry{hashedInfoData.find(SUPPORTED_VERSIONS_STR)};
        vector<string> versionStrs;
        if (supportedVersionsEntry) versionStrs =  PConf::valueAsList(supportedVersionsEntry->value);
        for (const auto& verStr : versionStrs) {
            Utils::Version supportedVersion{verStr};
            if (supportedVersion.err) {
                logger.warn("Prop " + propName + " lists invalid supported version: " + static_cast<string>(supportedVersion));
                continue;
            }

            logger.verbose("Prop " + propName + " supports OS version " + static_cast<string>(supportedVersion));
            std::unique_ptr<PCUI::VersionData> version;

            if (oldPropPtr) {
                for (auto& oldSupportedVersion : oldPropPtr->mSupportedVersions) {
                    if (oldSupportedVersion and static_cast<Utils::Version>(*oldSupportedVersion) == supportedVersion) {
                        version = std::move(oldSupportedVersion);
                        oldSupportedVersion = nullptr;
                    }
                }
            } 

            if (not version) version = std::make_unique<PCUI::VersionData>(supportedVersion);

            auto *versionedPropPtr{versionedProp.get()};
            version->setUpdateHandler([versionedPropPtr](uint32 id) {
                    if (id != PCUI::VersionData::ID_VALUE) return;

                    versionedPropPtr->saveInfo();
                    });
            supportedVersions.push_back(std::move(version));
        }

        // Yeah this naming is stupid, what are you going to do about it?
        auto dataFile{Paths::openInputFile(Paths::propDir() / propName / DATA_FILE_STR)};
        PConf::Data dataData;
        PConf::read(dataFile, dataData, logger.bverbose("Reading data file..."));
        const auto hashedDataData{PConf::hash(dataData)};

        auto prop{
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

vector<Versions::VersionedProp *> Versions::propsForVersion(const Utils::Version& version) {
    vector<VersionedProp *> ret;
    if (not propDefaultVersionMap.contains(version)) return {};

    ret.push_back(&propDefaultVersionMap.find(version)->second);
    auto [equalIter, equalEnd]{propVersionMap.equal_range(version)};
    for (; equalIter != equalEnd; ++equalIter) {
        ret.push_back(equalIter->second);
    }
    return ret;
}

optional<string> Versions::resetToDefault(bool purge, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Versions::resetToDefault()", lBranch)};

    const Utils::Version osVersion{7, 15};

    std::error_code err;
    if (purge) {
        logger.info("Purging versions...");
        fs::remove_all(Paths::versionDir(), err);
        fs::create_directories(Paths::versionDir(), err);
        if (err) {
            logger.error("Failed to create versions dir: " + err.message());
            return _("Failed during setup.").ToStdString();
        }
    }

    logger.info("Downloading ProffieOS...");
    auto uri{wxURI{
        Paths::remoteAssets() + "/ProffieOS/" +
            static_cast<string>(osVersion) + ".zip"
    }.BuildURI()};
    auto proffieOSRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
    auto requestResult{proffieOSRequest.Execute()};

    if (not requestResult) {
        logger.error("ProffieOS Download Failed\n" + requestResult.error.ToStdString());
        return _("Could not download ProffieOS").ToStdString();
    }

    wxZipInputStream osZipStream{*proffieOSRequest.GetResponse().GetStream()};
    if (not osZipStream.IsOk()) {
        logger.error("Could not open ProffieOS zip: " + std::to_string(osZipStream.GetLastError()));
        return _("Failed Opening ProffieOS ZIP").ToStdString();
    }

    std::unique_ptr<wxZipEntry> entry;
    constexpr cstring OS_EXTRACT_FAIL_MSG{wxTRANSLATE("Failed Extracting ProffieOS ZIP")};
    while (entry.reset(osZipStream.GetNextEntry()), entry) {
        auto filepath{Paths::os(osVersion) / entry->GetName().ToStdString()};
        fs::remove_all(filepath, err);
        if (filepath.string().find("__MACOSX") != string::npos) continue;

        if (entry->IsDir()) {
            if (not fs::exists(filepath, err)) {
                if (not fs::create_directories(filepath, err)) {
                    logger.error(
                        "Could not create dir " + filepath.string() +
                        ": " + err.message()
                    );
                    return wxGetTranslation(OS_EXTRACT_FAIL_MSG).ToStdString();
                }
            }
            continue;
        }

        if (not osZipStream.CanRead()) {
            logger.error("Failed reading ProffieOS: " + std::to_string(osZipStream.GetLastError()));
            return wxGetTranslation(OS_EXTRACT_FAIL_MSG).ToStdString();
        }

        wxFileOutputStream outStream{filepath.string()};
        if (not outStream.IsOk()) {
            logger.error("Failed writing ProffieOS: " + std::to_string(outStream.GetLastError()));
            return wxGetTranslation(OS_EXTRACT_FAIL_MSG).ToStdString();
        }

        osZipStream.Read(outStream);

        // TODO
        // auto permissionBits{entry->GetMode()};
        // fs::permissions(
        //     filepath,
        //     fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        //     fs::perm_options::add,
        //     err
        // );
    }

    if (osZipStream.GetLastError() != wxSTREAM_EOF) {
        logger.error("ProffieOS extraction finished with error: " + std::to_string(osZipStream.GetLastError()));
        return wxGetTranslation(OS_EXTRACT_FAIL_MSG).ToStdString();
    }

    logger.info("Downloading prop manifest...");
    uri = wxURI{Paths::remoteAssets() + "/props/manifest.pconf"}.BuildURI();
    auto propManifestRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
    requestResult = propManifestRequest.Execute();

    if (not requestResult) {
        logger.error("Prop Manifest Download Failed\n" + requestResult.error.ToStdString());
        return _("Could not download prop manifest").ToStdString();
    }

    std::istringstream manifestStream{
        propManifestRequest.GetResponse().AsString().ToStdString()
    };

    PConf::Data propManifestData;
    PConf::read(manifestStream, propManifestData, logger.binfo("Reading prop manifest file..."));
    bool propBundleExistsForVersion{false};
    for (const auto& entry : propManifestData) {
        if (entry->name!= "BUNDLES") continue;

        auto bundleStrings{PConf::valueAsList(entry->value)};
        for (const auto& bundleString : bundleStrings) {
            if (osVersion == Utils::Version{bundleString}) {
                propBundleExistsForVersion = true;
                break;
            }
        }

        if (propBundleExistsForVersion) break;
    }

    if (not propBundleExistsForVersion) {
        logger.error("Server reports no bundle for os version.");
        return _("Server Error").ToStdString();
    }

    uri = wxURI{
        Paths::remoteAssets() + "/props/bundles/" +
            static_cast<string>(osVersion) + ".zip"
    }.BuildURI();
    auto propBundleRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
    requestResult = propBundleRequest.Execute();

    if (not requestResult) {
        logger.error("Bundle Download Failed\n" + requestResult.error.ToStdString());
        return _("Could not download prop bundle").ToStdString();
    }

    wxZipInputStream propBundleZipStream(*propBundleRequest.GetResponse().GetStream());
    if (not propBundleZipStream.IsOk()) {
        logger.error("Could not open prop bundle zip: " + std::to_string(propBundleZipStream.GetLastError()));
        return _("Failed Opening Prop Bundle").ToStdString();
    }

    constexpr cstring BUNDLE_EXTRACT_FAIL_MSG{wxTRANSLATE("Failed Extracting Prop Bundle")};
    while (entry.reset(propBundleZipStream.GetNextEntry()), entry) {
        auto filepath{Paths::propDir() / entry->GetName().ToStdString()};
        fs::remove_all(filepath, err);
        if (filepath.string().find("__MACOSX") != string::npos) continue;

        if (entry->IsDir()) {
            if (not fs::exists(filepath, err)) {
                if (not fs::create_directories(filepath, err)) {
                    logger.error(
                        "Could not create dir " + filepath.string() +
                        ": " + err.message()
                    );
                    return wxGetTranslation(BUNDLE_EXTRACT_FAIL_MSG).ToStdString();
                }
            }
            continue;
        }

        if (not propBundleZipStream.CanRead()) {
            logger.error("Failed reading prop bundle: " + std::to_string(propBundleZipStream.GetLastError()));
            return wxGetTranslation(BUNDLE_EXTRACT_FAIL_MSG).ToStdString();
        }

        wxFileOutputStream outStream{filepath.string()};
        if (not outStream.IsOk()) {
            logger.error("Failed writing prop bundle: " + std::to_string(outStream.GetLastError()));
            return wxGetTranslation(BUNDLE_EXTRACT_FAIL_MSG).ToStdString();
        }

        propBundleZipStream.Read(outStream);
    }

    if (propBundleZipStream.GetLastError() != wxSTREAM_EOF) {
        logger.error("Bundle extraction finished with error: " + std::to_string(propBundleZipStream.GetLastError()));
        return wxGetTranslation(BUNDLE_EXTRACT_FAIL_MSG).ToStdString();
    }

    loadLocal();
    return nullopt;
}

