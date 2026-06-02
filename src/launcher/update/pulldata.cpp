#include "pulldata.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/pulldata.cpp
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

#include <thread>
#include <fstream>
#include <future>
#include <unordered_set>

#include <wx/webrequest.h>
#include <wx/utils.h>
#include <wx/richmsgdlg.h>

#include "log/logger.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/utils.hpp"
#include "utils/files.hpp"
#include "utils/hash.hpp"
#include "utils/paths.hpp"
#include "pconf/read.hpp"
#include "pconf/write.hpp"
#include "pconf/utils.hpp"

#include "update.hpp"

namespace {

/**
 * @return True if a fatal message was found.
 */
[[nodiscard]] bool checkMessages(const pconf::HashedData&, logging::Branch&);

[[nodiscard]] std::map<Update::ItemID, Update::Item> findItems(
    const pconf::HashedData&, logging::Branch&
);

[[nodiscard]] std::optional<std::pair<std::string, Update::Item>> parseItem(
    const pconf::EntryPtr&, logging::Logger&
);

[[nodiscard]] Update::Bundles resolveBundles(
    const pconf::HashedData&, logging::Branch&
);

void verifyBundles(
    const std::map<Update::ItemID, Update::Item>& items,
    Update::Bundles& bundles,
    logging::Branch&
);

} // namespace

bool Update::pullData(pcui::ProgressDialog *prog, logging::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::pullData()")};

    std::string errorMessage;

    bool requestComplete{false};
    auto handleRequestEvent{[&](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxCopyFile(evt.GetDataFile(), manifestFile().native());
        }

        switch (evt.GetState()) {
            case wxWebRequestBase::State_Failed:
            case wxWebRequestBase::State_Unauthorized:
                errorMessage = evt.GetErrorDescription().utf8_string();
                [[fallthrough]];
            case wxWebRequestBase::State_Completed:
            case wxWebRequestBase::State_Cancelled:
                requestComplete = true;
            default: break;
        }
    }};

    auto stateFile{files::openInput(paths::stateFile())};
    pconf::Data stateFileData;
    pconf::read(stateFile, stateFileData, logger.binfo("Reading manifest to fetch..."));
    const auto hashedStateFileData{pconf::hash(stateFileData)};
    const auto updateManifestEntry{hashedStateFileData.find("UPDATE_MANIFEST")};
    auto pullFrom{paths::remoteUpdateAssets()};
    if (updateManifestEntry and updateManifestEntry->value_) {
        pullFrom += "/manifest-" + *updateManifestEntry->value_ + ".pconf";
    } else {
        pullFrom += "/manifest.pconf";
    }

    auto webSession{wxWebSession::New()};
    auto request{webSession.CreateRequest(getEventHandler(), pullFrom)};
    request.SetStorage(wxWebRequest::Storage_File);
    getEventHandler()->Bind(wxEVT_WEBREQUEST_STATE, handleRequestEvent);

    constexpr cstring MSG{"Pulling version data..."};
    logger.info(MSG);
    prog->pulse(MSG);
    request.Start();

    while (not requestComplete) {
        prog->pulse();

        if (prog->cancelled()) {
            logger.info("Canceled.");
            return false;
        }

        wxYield();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    getEventHandler()->Unbind(wxEVT_WEBREQUEST_STATE, handleRequestEvent);

    if (request.GetState() != wxWebRequestSync::State_Completed) {
        const auto statusCode{request.GetResponse().GetStatus()};
        logger.warn("Data pull failed (" + std::to_string(statusCode) + "): " + errorMessage);
        if (statusCode == 404) {
            prog->finish(
                true,
                "Manifest File Invalid" "\n\n"
                "Check if a custom update channel is being used.\n"
                "If not, this is a bug, please contact me."
            );
        }
        return false;
    } 

    logger.info("Data pull completed with status " + std::to_string(request.GetResponse().GetStatus()));
    return true;
}

std::optional<Update::Data> Update::parseData(
    pcui::ProgressDialog *prog, logging::Branch& lBranch, bool heedMessages
) {
    auto& logger{lBranch.createLogger("Update::pullData()")};

    prog->set(20, "Parsing manifest...");

    if (prog->cancelled()) {
        logger.info("Canceled");
        return std::nullopt;
    }

    auto stream{files::openInput(manifestFile())};
    pconf::Data rawData;
    pconf::read(stream, rawData, logger.binfo("Parsing manifest..."));
    stream.close();
    auto hashedRawData{pconf::hash(rawData)};

    if (heedMessages) {
        prog->set(30, "Checking for messages...");

        if (prog->cancelled()) {
            logger.info("Canceled.");
            return std::nullopt;
        }

        if (checkMessages(hashedRawData, *logger.binfo("Checking Messages..."))) {
            logger.info("Canceling due to critical message.");
            return std::nullopt;
        }
    }

    prog->set(50, "Finding items...");

    if (prog->cancelled()) {
        logger.info("Canceled.");
        return std::nullopt;
    }

    auto items{findItems(hashedRawData, *logger.binfo("Finding items..."))};

    prog->set(60, "Resolving bundles...");

    if (prog->cancelled()) {
        logger.info("Canceled");
        return std::nullopt;
    }
    auto bundles{resolveBundles(hashedRawData, *logger.binfo("Resolving bundles..."))};

    prog->set(80, "Verifying bundles...");

    if (prog->cancelled()) {
        logger.info("Canceled");
        return std::nullopt;
    }
    verifyBundles(items, bundles, *logger.binfo("Verifying bundles..."));

    return Data{ .items=std::move(items), .bundles=std::move(bundles) };
}

namespace {

bool checkMessages(
    const pconf::HashedData& hashedRawData, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::checkMessages()")};

    utils::Version launcherVersion{wxSTRINGIZE(BIN_VERSION)};
    if (not launcherVersion) {
        logger.error("This launcher's version is invalid (" + static_cast<std::string>(launcherVersion) + "), messages will not be available!!");
        return false;
    }

    constexpr cstring IGNORED_MESSAGES_STR{"IGNORED_MESSAGES"};
    pconf::Data stateData;
    pconf::EntryPtr ignoreMessageEntry;
    std::vector<std::string> ignoreMessageList;
    std::unordered_set<std::string> ignoredMessages;

    auto stateFile{files::openInput(paths::stateFile())};
    pconf::read(stateFile, stateData, logger.bdebug("Parsing state file..."));
    stateFile.close();
    for (const auto& entry : stateData) {
        if (entry->name_ != IGNORED_MESSAGES_STR or not entry->value_) {
            continue;
        }

        ignoreMessageList = pconf::valueAsList(entry->value_);
        ignoredMessages = {
            ignoreMessageList.begin(),
            ignoreMessageList.end()
        };
        ignoreMessageEntry = entry;
        break;
    }

    bool hadFatal{false};
    const auto messageEntries{hashedRawData.findAll("MESSAGE")};
    for (const auto& messageEntry : messageEntries) {
        if (not messageEntry.section()) {
            logger.warn("Message entry found in pconf! (Not a section)");
            continue;
        }

        if (not messageEntry->label_) {
            logger.warn("Message without version specified is unacceptable.");
            continue;
        }

        auto hashedEntries{pconf::hash(messageEntry.section()->entries_)};
        auto contentEntry{hashedEntries.find("CONTENT")};
        if (not contentEntry) {
            logger.warn("Message missing content!");
            continue;
        }
        if (not contentEntry->value_) {
            logger.warn("Message content entry missing value!");
            continue;
        }
        bool isFatal{hashedEntries.find("FATAL")};

        auto label{*messageEntry->label_};
        Update::Comparator comp{};
        if (label.length() > 1) {
            if (label[0] == '<') comp = Update::Comparator::LESS_THAN;
            else if (label[0] == '>') comp = Update::Comparator::GREATER_THAN;
            else if (label[0] == '=' or std::isdigit(label[0])) comp = Update::Comparator::EQUAL;
            else {
                logger.warn("Message version invalid (comparator).");
                continue;
            }
        }
        if (not std::isdigit(label[0])) label = label.substr(1);
        utils::Version version{label};
        if (not version) {
            logger.warn("Failed to parse message version: " + static_cast<std::string>(version));
            continue;
        }

        bool messageApplicable{};
        switch (comp) {
            case Update::Comparator::EQUAL:
                messageApplicable = version.compare(launcherVersion) == 0;
                break;
            case Update::Comparator::LESS_THAN:
                // This looks backwards, because the `<>` in the pconf is
                // comparing to the launcher version (i.e. "launcher version
                // is less than"), but to maybe handle permissive versions
                // better we invert it evaluate permissive nums on the version
                // in pconf and then compare against launcherVersion. (i.e.
                // "greater than launcher version")
                //
                // Logically equivalent
                messageApplicable = version.compare(launcherVersion) > 0;
                break;
            case Update::Comparator::GREATER_THAN:
                messageApplicable = version.compare(launcherVersion) < 0;
                break;
        }
        if (not messageApplicable) continue;

        const auto idEntry{hashedEntries.find("ID")};
        if (idEntry and idEntry->value_) {
            if (ignoredMessages.contains(*idEntry->value_)) {
                logger.verbose("User ignored message " + *idEntry->value_ + "...");
                continue;
            }
        }

        std::promise<void> promise;

        const auto showMessage{[&] {
            auto res{pcui::showHideablePrompt(
                *contentEntry->value_,
                {
                    .caption_=isFatal
                        ? _("ProffieConfig Launcher Critical Notice")
                        : _("ProffieConfig Launcher Alert"),
                }
            )};

            if (res.wantsHide_) {
                ignoreMessageList.push_back(*contentEntry->value_);
            }

            promise.set_value();
        }};
        pcui::safeCall(showMessage);

        promise.get_future().get();

        if (isFatal) hadFatal = true;
    }

    if (not ignoreMessageEntry) ignoreMessageEntry = pconf::Section::create(IGNORED_MESSAGES_STR);
    ignoreMessageEntry->value_ = pconf::listAsValue(ignoreMessageList);
    const auto tmpPath{paths::stateFile().append(".tmp")};
    auto tmpFile{files::openOutput(tmpPath)};
    pconf::write(tmpFile, stateData, logger.bdebug("Writing statefile..."));
    tmpFile.close();
    std::error_code err;
    fs::rename(tmpPath, paths::stateFile(), err);

    return hadFatal;
}

std::map<Update::ItemID, Update::Item> findItems(
    const pconf::HashedData& rawHashedData,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::enumerateItems()")};

    std::map<Update::ItemID, Update::Item> ret;

    auto findType{[&logger, &ret](
        const std::vector<pconf::EntryPtr>& entries,
        Update::ItemType type,
        const std::string& typeStr
    ) {
        for (const auto& entry : entries) {
            auto parsed{parseItem(entry, logger)};
            if (parsed) {
                logger.info("Sucessfully parsed " + typeStr + ' ' + parsed->first);
                ret.emplace(
                    Update::ItemID{.type=type, .name=parsed->first},
                    parsed->second
                );
            }
        }
    }};

    findType(rawHashedData.findAll("EXEC"), Update::ItemType::EXEC, "executable");
    findType(rawHashedData.findAll("LIB"), Update::ItemType::LIB, "library");
    findType(rawHashedData.findAll("COMP"), Update::ItemType::COMP, "component");
    findType(rawHashedData.findAll("RSRC"), Update::ItemType::RSRC, "resource");

    return ret;
}

std::optional<std::pair<std::string, Update::Item>> parseItem(
    const pconf::EntryPtr& entry,
    logging::Logger& logger
) {
    if (not entry->label_) {
        logger.warn("Item missing name!");
        return std::nullopt; 
    }

    auto name{*entry->label_};

    if (not entry.section()) {
        logger.warn("Item \"" + name + "\" not a section!");
        return std::nullopt;
    }

#   ifdef _WIN32
    constexpr cstring PATH_KEY{"PATH_Win32"};
    constexpr cstring HASH_KEY{"HASH_Win32"};
#   elif defined(__APPLE__)
    constexpr cstring PATH_KEY{"PATH_macOS"};
    constexpr cstring HASH_KEY{"HASH_macOS"};
#   elif defined(__linux__)
    constexpr cstring PATH_KEY{"PATH_Linux"};
    constexpr cstring HASH_KEY{"HASH_Linux"};
#   endif

    auto hashedEntries{pconf::hash(entry.section()->entries_)};
    auto pathEntry{hashedEntries.find(PATH_KEY)};
    if (not pathEntry or not pathEntry->value_) {
        logger.info("Item \"" + name + "\" missing platform path, ignoring.");
        return std::nullopt;
    }

    Update::Item item;
    item.hidden = static_cast<bool>(hashedEntries.find("HIDDEN"));
    item.path = *pathEntry->value_;

    auto versionEntries{hashedEntries.findAll("VERSION")};
    for (const auto& versionEntry : versionEntries) {
        if (not versionEntry->label_) {
            logger.warn("Item \"" + name + "\" version unlabeled.");
            continue;
        }
        if (not versionEntry.section()) {
            logger.warn("Item \"" + name + "\" version not a section.");
            continue;
        }

        utils::Version version{*versionEntry->label_};
        if (not version) {
            std::string errMsg{"Item \""};
            errMsg += name;
            errMsg += "\" version \"";
            errMsg += *versionEntry->label_;
            errMsg += "\" invalid version str: " + *versionEntry->label_;
            logger.warn(errMsg);
            continue;
        }

        auto hashedVersionEntries{pconf::hash(versionEntry.section()->entries_)};

        auto versionHashEntry{hashedVersionEntries.find(HASH_KEY)};
        if (not versionHashEntry) {
            std::string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version "; 
            errMsg += version.string();
            errMsg += " does not have hash for this OS, skipping.";
            logger.info(errMsg);
            continue;
        }
        if (not versionHashEntry->value_) {
            std::string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += version.string();
            errMsg += " hash missing value.";
            logger.warn(errMsg);
            continue;
        }

        const auto hash{utils::hash::SHA256::parseString(
            *versionHashEntry->value_
        )};
        if (not hash) {
            std::string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += version.string();
            errMsg += " has invalid hash.";
            logger.warn(errMsg);
            continue;
        }

        Update::ItemVersionData versionData{.hash=*hash};

        auto fixEntries{hashedVersionEntries.findAll("FIX")};
        for (const auto& fixEntry : fixEntries) {
            if (not fixEntry->value_) {
                std::string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version "; 
                errMsg += version.string();
                errMsg += " fix missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.fixes.push_back(*fixEntry->value_);
        }

        auto changeEntries{hashedVersionEntries.findAll("CHANGE")};
        for (const auto& changeEntry : changeEntries) {
            if (not changeEntry->value_) {
                std::string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += version.string();
                errMsg += " change missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.changes.push_back(*changeEntry->value_);
        }

        auto featureEntries{hashedVersionEntries.findAll("FEAT")};
        for (const auto& featureEntry : featureEntries) {
            if (not featureEntry->value_) {
                std::string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += version.string();
                errMsg += " feature missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.features.push_back(*featureEntry->value_);
        }

        item.versions.emplace(version, versionData);
    }
     
    return std::pair{ name, item };
}

Update::Bundles resolveBundles(
    const pconf::HashedData& hashedRawData, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::resolveBundles()")};

    Update::Bundles ret;

    auto bundleEntries{hashedRawData.findAll("BUNDLE")};
    for (const auto& bundleEntry : bundleEntries) {
        if (not bundleEntry->label_) {
            logger.warn("Bundle unlabeled.");
            continue;
        }
        if (not bundleEntry.section()) {
            logger.warn("Bundle \"" + bundleEntry->label_.value() + "\" not a section.");
            continue;
        }

        utils::Version version{*bundleEntry->label_};
        if (not version) {
            logger.warn("Bundle \"" + *bundleEntry->label_ + "\" version invalid: " + version.string());
            continue;
        }

        auto hashedEntries{pconf::hash(bundleEntry.section()->entries_)};
        Update::Bundle bundle;

        auto noteEntry{hashedEntries.find("NOTE")};
        if (noteEntry) {
            if (not noteEntry->value_) logger.warn("Bundle \"" + version.string() + "\" note missing value");
            else bundle.note = noteEntry->value_.value();
        }

        auto parseReqItem{[&logger](
            const std::shared_ptr<pconf::Entry>& item
        ) -> std::optional<std::pair<std::string, utils::Version>> {
            if (not item->label_) {
                logger.warn("Item is unlabeled");
                return std::nullopt;
            }
            if (not item->value_) {
                logger.warn("Item \"" + item->label_.value() + "\" is unversioned.");
                return std::nullopt;
            }

            utils::Version version{*item->value_};
            if (not version) {
                logger.warn("Item \"" + item->label_.value() + "\" version \"" + item->value_.value() + "\" is invalid: " + version.string());
                return std::nullopt;
            }

            return std::pair{ *item->label_, version };
        }};

        auto fillReqFiles{[&](
            const std::vector<pconf::EntryPtr>& entries, Update::ItemType type
        ) {
            for (const auto& entry : entries) {
                auto parsed{parseReqItem(entry)};
                if (parsed) {
                    logger.debug("Added to Bundle " + version.string() + ": " + parsed->first + ", " + parsed->second.string());
                    bundle.reqs.emplace_back(
                        Update::ItemID{.type=type, .name=parsed->first},
                        parsed->second
                    );
                }
            }
        }};

        fillReqFiles(hashedEntries.findAll("EXEC"), Update::ItemType::EXEC);
        fillReqFiles(hashedEntries.findAll("LIB"), Update::ItemType::LIB);
        fillReqFiles(hashedEntries.findAll("COMP"), Update::ItemType::COMP);
        fillReqFiles(hashedEntries.findAll("RSRC"), Update::ItemType::RSRC);

        logger.info("Parsed bundle " + version.string());
        ret.emplace(version, bundle);
    }

    return ret;
}

void verifyBundles(
    const std::map<Update::ItemID, Update::Item>& items,
    Update::Bundles& bundles,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::verifyBundles()")};

    for (auto bundleIt{bundles.begin()}; bundleIt != bundles.end();) {
        auto& [ version, bundle ]{*bundleIt};
        bool eraseBundle{false};

        for (auto& [ fileID, fileVer, hash] : bundle.reqs) {
            auto itemIt{items.find(fileID)};
            if (itemIt == items.end()) {
                std::string message{"Bundle "};
                message += version.string();
                message += " contains an item (";
                message += fileID.name + ':' + fileVer.string();
                message += ") which is not registered for this OS, ignoring the item.";
                logger.debug(message);
                fileID.ignored = true;
                continue;
            }

            auto itemVerIt{itemIt->second.versions.find(fileVer)};
            if (itemVerIt == itemIt->second.versions.end()) {
                std::string message{"Bundle "};
                message += version.string();
                message += " contains a version of an item (";
                message += fileID.name + ':' + fileVer.string();
                message += ") which is not registered for this OS, ignoring the bundle.";
                logger.debug(message);
                eraseBundle = true;
                break;
            }

            // Assign hash after verification
            hash = itemVerIt->second.hash;
        }

        if (eraseBundle) bundleIt = bundles.erase(bundleIt);
        else ++bundleIt;
    }
}

} // namespace

