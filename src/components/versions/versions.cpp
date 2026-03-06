#include "versions.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/versions/versions.cpp
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
#include <memory>
#include <sstream>

#include <wx/uri.h>
#include <wx/webrequest.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "log/context.hpp"
#include "log/logger.hpp"
#include "pconf/utils.hpp"
#include "pconf/read.hpp"
#include "pconf/write.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/types.hpp"
#include "utils/version.hpp"
#include "versions/detail/boards.hpp"
#include "versions/detail/strings.hpp"
#include "versions/os.hpp"
#include "versions/priv/data.hpp"
#include "versions/prop.hpp"

namespace {

constexpr utils::Version DEFAULT_OS_VERSION{7, 15};

std::optional<std::string> downloadOS(utils::Version, logging::Branch&);
std::optional<std::string> downloadProps(utils::Version, logging::Branch&);

} // namespace

void versions::loadLocal(logging::Branch *lBranch) {
    auto& logger{logging::Branch::optCreateLogger("Versions::loadLocal()", lBranch)};

    logger.info("Loading ProffieOS Versions...");
    priv::os.clear();
    for (const auto& entry : fs::directory_iterator(paths::osDir())) {
        std::error_code err{};
        if (not entry.is_directory(err)) {
            logger.warn("Non-directory OS entry found: " + entry.path().filename().string());
            continue;
        }

        utils::Version version{entry.path().filename().string()};
        if (not version or not version.isExact()) {
            logger.warn("OS dir with invalid version: " + entry.path().filename().string());
            continue;
        }

        bool duplicate{false};
        for (const auto& versionedOS : priv::os) {
            if (versionedOS->version_.compare(version) == 0) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            logger.warn("Duplicate os entry: " + static_cast<std::string>(version));
            continue;
        }

        logger.info("Found ProffieOS version " + static_cast<std::string>(version) + "...");

        auto infoFile{files::openInput(entry.path() / detail::INFO_FILE_STR)};
        pconf::Data infoData;
        if (not pconf::read(infoFile, infoData, logger.bverbose("Reading info file..."))) {
            logger.error("Could not read info pconf.");
            continue;
        }

        const auto hashedInfoData{pconf::hash(infoData)};

        const auto coreVersionEntry{hashedInfoData.find(detail::CORE_VER_STR)};
        if (not coreVersionEntry or not coreVersionEntry->value_) {
            logger.error("Missing core version entry.");
            continue;
        } 

        utils::Version coreVersion{*coreVersionEntry->value_};
        if (not coreVersion or not coreVersion.isExact()) {
            logger.error("Invalid core version entry.");
            continue;
        }

        const auto coreURLEntry{hashedInfoData.find(detail::CORE_URL_STR)};
        if (not coreURLEntry or not coreURLEntry->value_) {
            logger.error("Missing core url entry.");
            continue;
        } 

        auto& coreURL{*coreURLEntry->value_};

        std::vector<os::BoardInfo> boards;

        const auto boardEntries{hashedInfoData.findAll(detail::BOARD_STR)};
        for (const auto& boardEntry : boardEntries) {
            std::string include;
            std::string coreId;

            bool boardKnown{false};
            for (const auto& board : detail::BOARDS) {
                if (boardEntry->label_ != board.name_) continue;

                boardKnown = true;
                include = board.include_;
                coreId = board.coreId_;
            }

            if (not boardKnown) {
                logger.error("Invalid board entry.");
                continue;
            }

            if (auto boardSection{boardEntry.section()}) {
                const auto boardVars{pconf::hash(boardSection->entries_)};

                auto coreIdEntry{boardVars.find(detail::CORE_ID_STR)};
                if (coreIdEntry and coreIdEntry->value_) {
                    coreId = *coreIdEntry->value_;
                }

                auto includeEntry{boardVars.find(detail::INCLUDE_STR)};
                if (includeEntry and includeEntry->value_) {
                    include = *includeEntry->value_;
                }
            }

            boards.emplace_back(
                std::move(*boardEntry->label_),
                std::move(coreId),
                std::move(include)
            );
        }

        priv::os.push_back(std::make_unique<os::Versioned>(
            std::move(version),
            std::move(coreURL),
            std::move(coreVersion),
            std::move(boards)
        ));
    }

    logger.info("Loading Props...");

    priv::props.clear();
    for (const auto& entry : fs::directory_iterator(paths::propDir())) {
        std::error_code err{};
        if (not entry.is_directory(err)) {
            logger.warn("Non-directory prop entry found: " + entry.path().filename().string());
            continue;
        }

        auto propName{entry.path().filename().string()};
        uint32 numTrimmed{};
        utils::trim(
            propName,
            PROP_NAME_RULES,
            &numTrimmed,
            -1
        );
        if (numTrimmed) {
            logger.warn("Prop entry with invalid name: " + entry.path().filename().string());
            continue;
        }

        logger.info("Found prop " + propName + "...");

        auto infoFile{files::openInput(
            paths::propDir() / propName / detail::INFO_FILE_STR
        )};
        pconf::Data infoData;
        if (not pconf::read(infoFile, infoData, logger.bverbose("Reading info file..."))) {
            logger.error("Could not read info pconf.");
            continue;
        }

        const auto hashedInfoData{pconf::hash(infoData)};

        const auto supportedVersionsEntry{
            hashedInfoData.find(detail::SUPPORTED_VERSIONS_STR)
        };
        if (not supportedVersionsEntry) {
            logger.error("Prop missing supported versions.");
            continue;
        }
        std::vector<std::string> versionStrs{pconf::valueAsList(
            supportedVersionsEntry->value_
        )};

        std::vector<utils::Version> versions;

        for (const auto& verStr : versionStrs) {
            utils::Version version{verStr};
            if (not version) {
                logger.warn("Prop " + propName + " lists invalid supported version: " + static_cast<std::string>(version));
                continue;
            }

            logger.verbose("Prop " + propName + " supports OS version " + static_cast<std::string>(version));
            versions.push_back(std::move(version));
        }

        // Yeah this naming is stupid, what are you going to do about it?
        auto dataFile{files::openInput(
            paths::propDir() / propName / detail::DATA_FILE_STR
        )};
        pconf::Data dataData;
        if (not pconf::read(dataFile, dataData, logger.bverbose("Reading data file..."))) {
            logger.error("Cannot read data file for " + propName);
            continue;
        }
        const auto hashedDataData{pconf::hash(dataData)};

        auto prop{props::Prop::generate(
            hashedDataData, logger.bverbose("Generating prop...")
        )};
        if (not prop) {
            logger.error("Failed generating prop " + propName);
            continue;
        }

        priv::props.push_back(std::make_unique<props::Versioned>(
            std::move(propName),
            std::move(versions),
            std::move(prop)
        ));
    }

    logger.info("Done");
}

std::optional<std::string> versions::fetch(logging::Branch *lBranch) {
    auto& logger{logging::Branch::optCreateLogger("versions::fetch()", lBranch)};

    wxURI uri;
    wxWebRequestSync request;
    wxWebRequestSync::Result result;
    std::istringstream stream;
    pconf::Data data;

    logger.info("Downloading prop manifest...");
    uri = paths::remoteAssets() + "/props/manifest.pconf";
    request = wxWebSessionSync::GetDefault().CreateRequest(
        uri.BuildURI()
    );

    result = request.Execute();

    if (not result) {
        logger.error("Prop Manifest Download Failed\n" + result.error.ToStdString());
        return _("Could not download prop manifest").ToStdString();
    }

    stream.str(request.GetResponse().AsString().ToStdString());

    if (not pconf::read(stream, data, logger.binfo("Reading prop manifest file..."))) {
        logger.error("Prop Manifest Parse Failed.");
        return _("Could not parse prop manifest").ToStdString();
    }

    { std::lock_guard scopeLock{priv::lock};
        priv::availableProps.clear();
        for (const auto& entry : data) {
            if (entry->name_ != detail::PROP_STR) continue;
            auto section{entry.section()};
            if (not section) continue;
            if (not section->label_) {
                logger.warn("Prop entry missing name.");
                continue;
            }

            auto name{*section->label_};

            uint32 numTrimmed{};
            utils::TrimRules rules{
                .allowAlpha=true,
                .allowNum=true,
                .safeList="_-"
            };
            utils::trim(name, rules, &numTrimmed);

            if (numTrimmed) {
                logger.warn("Prop entry invalid name: " + name);
                continue;
            }

            auto hashedPropEntries{pconf::hash(section->entries_)};

            auto supportedVersionsEntry{hashedPropEntries.find(
                detail::SUPPORTED_VERSIONS_STR
            )};
            if (
                    not supportedVersionsEntry or
                    not supportedVersionsEntry->value_
               ) {
                logger.warn("Prop " + name + " missing supported versions.");
                continue;
            }

            auto supportedVersionsStrs{pconf::valueAsList(
                supportedVersionsEntry->value_
            )};

            std::vector<utils::Version> supportedVersions;
            for (const auto& suppVerStr : supportedVersionsStrs) {
                utils::Version ver{suppVerStr};
                if (not ver) {
                    logger.warn("Prop " + name + " invalid supported version: " += suppVerStr);
                    continue;
                }

                supportedVersions.push_back(std::move(ver));
            }

            priv::availableProps.push_back(props::Available{
                .name_ = name,
                .supportedVersions_ = std::move(supportedVersions),
            });
        }
    }

    logger.info("Downloading ProffieOS manifest...");
    uri = paths::remoteAssets() + "/ProffieOS/manifest.pconf";
    request = wxWebSessionSync::GetDefault().CreateRequest(
        uri.BuildURI()
    );

    result = request.Execute();

    if (not result) {
        logger.error("ProffieOS Manifest Download Failed\n" + result.error.ToStdString());
        return _("Could not download ProffieOS manifest").ToStdString();
    }

    stream.str(request.GetResponse().AsString().ToStdString());

    if (not pconf::read(stream, data, logger.binfo("Reading ProffieOS manifest file..."))) {
        logger.error("ProffieOS Manifest Parse Failed.");
        return _("Could not parse ProffieOS manifest").ToStdString();
    }

    { std::lock_guard scopeLock{priv::lock};
        priv::availableOS.clear();
        for (const auto& entry : data) {
            if (entry->name_ != detail::OS_STR) continue;
            auto section{entry.section()};
            if (not section) continue;
            if (not section->label_) {
                logger.warn("ProffieOS entry missing version");
                continue;
            }

            const auto& verStr{*section->label_};
            utils::Version ver{verStr};

            if (not ver or not ver.isExact()) {
                logger.warn("ProffieOS entry invalid version: " + verStr);
                continue;
            }

            auto propEntries{pconf::hash(section->entries_)};

            std::string coreUrl{"https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json"};
            if (auto coreUrlEntry{propEntries.find(detail::CORE_URL_STR)}) {
                if (coreUrlEntry->value_) {
                    if (not wxURI{*coreUrlEntry->value_}.IsReference()) {
                        logger.warn("ProffieOS " + verStr + " invalid coreURL: " + *coreUrlEntry->value_);
                    } else coreUrl = *coreUrlEntry->value_;
                }
            }

            utils::Version coreVersion{3, 6};
            if (auto coreVerEntry{propEntries.find(detail::CORE_VER_STR)}) {
                if (coreVerEntry->value_) {
                    utils::Version ver{*coreVerEntry->value_};
                    if (not ver or not ver.isExact()) {
                        logger.warn("ProffieOS " + verStr + " invalid core version: " + *coreVerEntry->value_);
                    } else coreVersion = ver;
                }
            }

            std::vector<os::Available::BoardInfo> boards;
            auto boardEntries{propEntries.findAll(detail::BOARD_STR)};
            for (auto& boardEntry : boardEntries) {
                if (not boardEntry->label_) {
                    logger.warn("ProffieOS " + verStr + " board version missing");
                    continue;
                }

                std::string include;
                std::string coreId;

                bool boardKnown{false};
                for (const auto& board : detail::BOARDS) {
                    if (boardEntry->label_ != board.name_) continue;

                    boardKnown = true;
                    include = board.include_;
                    coreId = board.coreId_;
                }

                if (not boardKnown) {
                    logger.error("Invalid board entry.");
                    continue;
                }

                if (auto boardSection{boardEntry.section()}) {
                    const auto boardVars{pconf::hash(boardSection->entries_)};

                    auto coreIdEntry{boardVars.find(detail::CORE_ID_STR)};
                    if (coreIdEntry and coreIdEntry->value_) {
                        coreId = *coreIdEntry->value_;
                    }

                    auto includeEntry{boardVars.find(detail::INCLUDE_STR)};
                    if (includeEntry and includeEntry->value_) {
                        include = *includeEntry->value_;
                    }
                }

                boards.push_back(os::Available::BoardInfo{
                    .name_=std::move(*boardEntry->label_),
                    .coreId_=std::move(coreId),
                    .include_=std::move(include),
                });
            }

            priv::availableOS.push_back(os::Available{
                .version_=std::move(ver),
                .coreUrl_=std::move(coreUrl),
                .coreVersion_=std::move(coreVersion),
                .boards_=std::move(boards),
            });
        }
    }

    return std::nullopt;
}

std::optional<std::string> versions::installDefault(
    bool purge, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("Versions::resetToDefault()", lBranch)};

    std::error_code err;
    if (purge) {
        logger.info("Purging versions...");
        fs::remove_all(paths::versionDir(), err);
        fs::create_directories(paths::versionDir(), err);
        if (err) {
            logger.error("Failed to create versions dir: " + err.message());
            return _("Failed during setup.").ToStdString();
        }
    }

    auto osDownErr{downloadOS(DEFAULT_OS_VERSION, logger.binfo("Downloading ProffieOS"))};
    if (osDownErr) return osDownErr;

    { std::lock_guard scopeLock{priv::lock};
        for (const auto& availProp : priv::availableProps) {
            bool supportsDefault{false};
            for (const auto& ver : availProp.supportedVersions_) {
                if (ver.compare(DEFAULT_OS_VERSION) != 0) continue;

                supportsDefault = true;
                break;
            }

            auto downErr{downloadProp(availProp.name_, logger.binfo("Downloading Prop " + availProp.name_))};
            if (downErr) return downErr;
        }
    }

    return std::nullopt;
}


std::optional<std::string> versions::downloadOS(
    const utils::Version& ver, logging::Branch *lBranch
) {
    std::lock_guard scopeLock{priv::lock};

    auto& logger{logging::Branch::optCreateLogger("versions::downloadOS()", lBranch)};

    bool known{false};
    const os::Available *info{};
    for (const auto& avail : priv::availableOS) {
        if (avail.version_.compare(ver) != 0) continue;

        known = true;
        info = &avail;
        break;
    }

    if (not known) {
        logger.error("Unknown ProffieOS version: " + static_cast<std::string>(ver));
        return _("Unknown ProffieOS Version").ToStdString();
    }

    logger.info("Downloading ProffieOS...");
    wxURI uri{
        paths::remoteAssets() + "/ProffieOS/" +
        static_cast<std::string>(ver) + ".zip"
    };
    auto proffieOSRequest{wxWebSessionSync::GetDefault().CreateRequest(
        uri.BuildURI()
    )};
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

    std::error_code ec;
    
    std::unique_ptr<wxZipEntry> entry;
    constexpr cstring OS_EXTRACT_FAIL_MSG{wxTRANSLATE("Failed Extracting ProffieOS ZIP")};
    while (entry.reset(osZipStream.GetNextEntry()), entry) {
        auto filepath{
            paths::osDir() /
            static_cast<std::string>(ver) /
            entry->GetName().ToStdString()
        };
        fs::remove_all(filepath, ec);
        if (filepath.string().find("__MACOSX") != std::string::npos) continue;

        if (entry->IsDir()) {
            if (not fs::exists(filepath, ec)) {
                // Return value is just if newly created, not success
                fs::create_directories(filepath, ec);
                if (ec) {
                    logger.error(
                        "Could not create dir " + filepath.string() +
                        ": " + ec.message()
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
    }

    if (osZipStream.GetLastError() != wxSTREAM_EOF) {
        logger.error("ProffieOS extraction finished with error: " + std::to_string(osZipStream.GetLastError()));
        return wxGetTranslation(OS_EXTRACT_FAIL_MSG).ToStdString();
    }

    pconf::Data data;
    data.push_back(pconf::Entry::create(
        detail::CORE_URL_STR, info->coreUrl_
    ));
    data.push_back(pconf::Entry::create(
        detail::CORE_VER_STR, info->coreVersion_
    ));

    for (const auto& board : info->boards_) {
        auto sect{pconf::Section::create(detail::BOARD_STR, board.name_)};

        sect->entries_.push_back(
            pconf::Entry::create(detail::CORE_ID_STR, board.coreId_)
        );
        sect->entries_.push_back(
            pconf::Entry::create(detail::INCLUDE_STR, board.include_)
        );

        data.emplace_back(std::move(sect));
    }

    auto fstream{files::openOutput(
        paths::osDir() / static_cast<std::string>(ver) / detail::INFO_FILE_STR
    )};

    pconf::write(fstream, data, logger.binfo("Writing ProffieOS info file..."));

    if (fstream.fail()) {
        logger.error("Info file write failed: " + std::to_string(fstream.rdstate()));
        return _("Could not write ProffieOS info file").ToStdString();
    }

    return std::nullopt;
}

std::optional<std::string> versions::downloadProp(
    const std::string& name, logging::Branch *lBranch
) {
    std::lock_guard scopeLock{priv::lock};

    auto& logger{logging::Branch::optCreateLogger("versions::downloadProp()", lBranch)};

    bool known{false};
    const props::Available *info{};
    for (const auto& avail : priv::availableProps) {
        if (avail.name_ != name) continue;

        known = true;
        info = &avail;
        break;
    }

    if (not known) {
        logger.error("Unknown prop: " + name);
        return _("Unknown Prop File").ToStdString();
    }

    bool copyRes{};
    std::error_code ec;
    wxWebRequestSync request;
    wxWebRequestSync::Result result;

    fs::create_directories(paths::propDir() / name, ec);

    wxURI baseURI{paths::remoteAssets() + "/props/" + name};
    
    wxURI dataURI{detail::DATA_FILE_STR};
    dataURI.Resolve(baseURI);

    request = wxWebSessionSync::GetDefault().CreateRequest(
        dataURI.BuildURI()
    );
    request.SetStorage(wxWebRequestSync::Storage::Storage_File);
    result = request.Execute();

    if (not result) {
        logger.error("Prop data download failed\n" + result.error.ToStdString());
        return _("Could not download prop").ToStdString();
    }

    copyRes = files::copyOverwrite(
        request.GetResponse().GetDataFile().ToStdString(),
        paths::propDir() / name / detail::DATA_FILE_STR,
        ec
    );

    if (not copyRes) {
        logger.error("Failed to copy data file: " + ec.message());
        return _("Could not copy prop data file").ToStdString();
    }

    wxURI headerURI{detail::HEADER_FILE_STR};
    headerURI.Resolve(baseURI);

    request = wxWebSessionSync::GetDefault().CreateRequest(
        headerURI.BuildURI()
    );
    request.SetStorage(wxWebRequestSync::Storage::Storage_File);
    result = request.Execute();

    if (not result) {
        logger.error("Prop header download failed\n" + result.error.ToStdString());
        return _("Could not download prop").ToStdString();
    }

    copyRes = files::copyOverwrite(
        request.GetResponse().GetDataFile().ToStdString(),
        paths::propDir() / name / detail::HEADER_FILE_STR,
        ec
    );

    if (not copyRes) {
        logger.error("Failed to copy header file: " + ec.message());
        return _("Could not copy prop header file").ToStdString();
    }

    pconf::Data data;
    
    std::vector<std::string> verStrs;
    verStrs.reserve(info->supportedVersions_.size());
    for (const auto& ver : info->supportedVersions_) verStrs.push_back(ver);

    data.push_back(pconf::Entry::create(
        detail::SUPPORTED_VERSIONS_STR, pconf::listAsValue(verStrs)
    ));

    auto fstream{files::openOutput(
        paths::propDir() / name / detail::INFO_FILE_STR
    )};

    pconf::write(fstream, data, logger.binfo("Writing prop info file..."));

    if (fstream.fail()) {
        logger.error("Info file write failed: " + std::to_string(fstream.rdstate()));
        return _("Could not write prop info file").ToStdString();
    }

    return std::nullopt;
}

