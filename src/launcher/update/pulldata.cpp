#include "pulldata.h"
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
#include <unordered_set>

#include <wx/webrequest.h>
#include <wx/utils.h>
#include <wx/richmsgdlg.h>

#include "log/logger.h"
#include "utils/paths.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"

#include "update.h"

namespace {

/**
 * @return True if a fatal message was found.
 */
[[nodiscard]] bool checkMessages(const PConf::HashedData&, Log::Branch&);

[[nodiscard]] std::map<Update::ItemID, Update::Item> findItems(const PConf::HashedData&, Log::Branch&);

[[nodiscard]] optional<std::pair<string, Update::Item>> parseItem(const PConf::EntryPtr&, Log::Logger&);

[[nodiscard]] std::map<Utils::Version, Update::Bundle> resolveBundles(const PConf::HashedData&, Log::Branch&);

void verifyBundles(const std::map<Update::ItemID, Update::Item>& items, std::map<Utils::Version, Update::Bundle>& bundles, Log::Branch&);

} // namespace

bool Update::pullData(PCUI::ProgressDialog *prog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::pullData()")};

    bool requestComplete{false};
    auto handleRequestEvent{[&](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxCopyFile(evt.GetDataFile(), manifestFile().native());
        }

        switch (evt.GetState()) {
            case wxWebRequestBase::State_Unauthorized:
            case wxWebRequestBase::State_Completed:
            case wxWebRequestBase::State_Failed:
            case wxWebRequestBase::State_Cancelled:
                requestComplete = true;
            default: break;
        }
    }};

    std::ifstream stateFile{Paths::stateFile()};
    PConf::Data stateFileData;
    PConf::read(stateFile, stateFileData, logger.binfo("Reading manifest to fetch..."));
    const auto hashedStateFileData{PConf::hash(stateFileData)};
    const auto updateManifestEntry{hashedStateFileData.find("UPDATE_MANIFEST")};
    auto pullFrom{Paths::remoteUpdateAssets()};
    if (updateManifestEntry and updateManifestEntry->value) {
        pullFrom += "/manifest-" + *updateManifestEntry->value + ".pconf";
    } else {
        pullFrom += "/manifest.pconf";
    }

    auto webSession{wxWebSession::New()};
    auto request{webSession.CreateRequest(getEventHandler(), pullFrom)};
    request.SetStorage(wxWebRequest::Storage_File);
    getEventHandler()->Bind(wxEVT_WEBREQUEST_STATE, handleRequestEvent);

    constexpr cstring MSG{"Pulling version data..."};
    logger.info(MSG);
    prog->Pulse(MSG);
    request.Start();

    while (not requestComplete) {
        if (not prog->Pulse()) {
            logger.info("Canceled.");
            return false;
        }

        wxYield();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    getEventHandler()->Unbind(wxEVT_WEBREQUEST_STATE, handleRequestEvent);

    if (request.GetState() != wxWebRequestSync::State_Completed) {
        const auto statusCode{request.GetResponse().GetStatus()};
        logger.warn("Data pull failed: " + std::to_string(statusCode));
        if (statusCode == 404) {
            PCUI::showMessage("Check if a custom update channel is being used.\n\nIf not, this is a bug, please contact me.", "Manifest File Invalid");
        }
        return false;
    } 

    logger.info("Data pull completed with status " + std::to_string(request.GetResponse().GetStatus()));
    return true;
}

optional<Update::Data> Update::parseData(PCUI::ProgressDialog *prog, Log::Branch& lBranch, bool heedMessages) {
    auto& logger{lBranch.createLogger("Update::pullData()")};

    if (not prog->Update(20, "Parsing manifest...")) {
        logger.info("Canceled");
        return nullopt;
    }

    std::ifstream stream{manifestFile()};
    PConf::Data rawData;
    PConf::read(stream, rawData, logger.binfo("Parsing manifest..."));
    stream.close();
    auto hashedRawData{PConf::hash(rawData)};

    if (heedMessages) {
        if (not prog->Update(30, "Checking for messages...")) {
            logger.info("Canceled.");
            return nullopt;
        }
        if (checkMessages(hashedRawData, *logger.binfo("Checking Messages..."))) {
            logger.info("Canceling due to critical message.");
            return nullopt;
        }
    }

    if (not prog->Update(50, "Finding items...")) {
        logger.info("Canceled.");
        return nullopt;
    }
    auto items{findItems(hashedRawData, *logger.binfo("Finding items..."))};

    if (not prog->Update(60, "Resolving bundles...")) {
        logger.info("Canceled");
        return nullopt;
    }
    auto bundles{resolveBundles(hashedRawData, *logger.binfo("Resolving bundles..."))};

    if (not prog->Update(80, "Verifying bundles...")) {
        logger.info("Canceled");
        return nullopt;
    }
    verifyBundles(items, bundles, *logger.binfo("Verifying bundles..."));

    return Data{ .items=std::move(items), .bundles=std::move(bundles) };
}

namespace {

bool checkMessages(const PConf::HashedData& hashedRawData, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::checkMessages()")};

    Utils::Version launcherVersion{wxSTRINGIZE(BIN_VERSION)};
    if (not launcherVersion) {
        logger.error("This launcher's version is invalid (" + static_cast<string>(launcherVersion) + "), messages will not be available!!");
        return false;
    }

    constexpr cstring IGNORED_MESSAGES_STR{"IGNORED_MESSAGES"};
    PConf::Data stateData;
    PConf::EntryPtr ignoreMessageEntry;
    vector<string> ignoreMessageList;
    std::unordered_set<string> ignoredMessages;

    std::ifstream stateFile{Paths::stateFile()};
    PConf::read(stateFile, stateData, logger.bdebug("Parsing state file..."));
    stateFile.close();
    for (const auto& entry : stateData) {
        if (entry->name != IGNORED_MESSAGES_STR or not entry->value) continue;

        ignoreMessageList = PConf::valueAsList(entry->value);
        ignoredMessages = {ignoreMessageList.begin(), ignoreMessageList.end()};
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

        if (not messageEntry->label) {
            logger.warn("Message without version specified is unacceptable.");
            continue;
        }

        auto hashedEntries{PConf::hash(messageEntry.section()->entries)};
        auto contentEntry{hashedEntries.find("CONTENT")};
        if (not contentEntry) {
            logger.warn("Message missing content!");
            continue;
        }
        if (not contentEntry->value) {
            logger.warn("Message content entry missing value!");
            continue;
        }
        bool isFatal{hashedEntries.find("FATAL")};

        auto label{*messageEntry->label};
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
        Utils::Version version{label};
        if (not version) {
            logger.warn("Failed to parse message version: " + static_cast<string>(version));
            continue;
        }

        bool messageApplicable{};
        switch (comp) {
            case Update::Comparator::EQUAL:
                messageApplicable = launcherVersion == version;
                break;
            case Update::Comparator::LESS_THAN:
                messageApplicable = launcherVersion < version;
                break;
            case Update::Comparator::GREATER_THAN:
                messageApplicable = launcherVersion > version;
                break;
        }
        if (not messageApplicable) continue;

        const auto idEntry{hashedEntries.find("ID")};
        if (idEntry and idEntry->value) {
            if (ignoredMessages.contains(*idEntry->value)) {
                logger.verbose("User ignored message " + *idEntry->value + "...");
                continue;
            }
        }

        wxRichMessageDialog dialog{
            nullptr,
            *contentEntry->value,
            isFatal ? _("ProffieConfig Launcher Critical Notice") : _("ProffieConfig Launcher Alert")
        };

        dialog.ShowCheckBox("Do Not Show Again");
        if (dialog.IsCheckBoxChecked()) {
            ignoreMessageList.push_back(*contentEntry->value);
        }

        if (isFatal) hadFatal = true;
    }

    if (not ignoreMessageEntry) ignoreMessageEntry = PConf::Section::create(IGNORED_MESSAGES_STR);
    ignoreMessageEntry->value = PConf::listAsValue(ignoreMessageList);
    const auto tmpPath{Paths::stateFile().append(".tmp")};
    std::ofstream tmpFile{tmpPath};
    PConf::write(tmpFile, stateData, logger.bdebug("Writing statefile..."));
    tmpFile.close();
    std::error_code err;
    fs::rename(tmpPath, Paths::stateFile(), err);

    return hadFatal;
}

std::map<Update::ItemID, Update::Item> findItems(
    const PConf::HashedData& rawHashedData,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::enumerateItems()")};

    std::map<Update::ItemID, Update::Item> ret;

    auto findType{[&logger, &ret](const vector<PConf::HashedData::IndexedEntryPtr>& entries, Update::ItemType type, const string& typeStr) {
        for (const auto& entry : entries) {
            auto parsed{parseItem(entry, logger)};
            if (parsed) {
                logger.info("Sucessfully parsed " + typeStr + ' ' + parsed->first);
                ret.emplace(Update::ItemID{ .type=type, .name=parsed->first }, parsed->second);
            }
        }
    }};

    findType(rawHashedData.findAll("EXEC"), Update::ItemType::EXEC, "executable");
    findType(rawHashedData.findAll("LIB"), Update::ItemType::LIB, "library");
    findType(rawHashedData.findAll("COMP"), Update::ItemType::COMP, "component");
    findType(rawHashedData.findAll("RSRC"), Update::ItemType::RSRC, "resource");

    return ret;
}

optional<std::pair<string, Update::Item>> parseItem(
    const PConf::EntryPtr& entry,
    Log::Logger& logger
) {
    if (not entry->label) {
        logger.warn("Item missing name!");
        return nullopt; 
    }

    auto name{*entry->label};

    if (not entry.section()) {
        logger.warn("Item \"" + name + "\" not a section!");
        return nullopt;
    }

#   ifdef __WIN32__
    constexpr cstring PATH_KEY{"PATH_Win32"};
    constexpr cstring HASH_KEY{"HASH_Win32"};
#   elif defined(__APPLE__)
    constexpr cstring PATH_KEY{"PATH_macOS"};
    constexpr cstring HASH_KEY{"HASH_macOS"};
#   elif defined(__linux__)
    constexpr cstring PATH_KEY{"PATH_Linux"};
    constexpr cstring HASH_KEY{"HASH_Linux"};
#   endif

    auto hashedEntries{PConf::hash(entry.section()->entries)};
    auto pathEntry{hashedEntries.find(PATH_KEY)};
    if (not pathEntry or not pathEntry->value) {
        logger.info("Item \"" + name + "\" missing platform path, ignoring.");
        return nullopt;
    }

    Update::Item item;
    item.hidden = static_cast<bool>(hashedEntries.find("HIDDEN"));
    item.path = *pathEntry->value;

    auto versionEntries{hashedEntries.findAll("VERSION")};
    for (const auto& versionEntry : versionEntries) {
        if (not versionEntry->label) {
            logger.warn("Item \"" + name + "\" version unlabeled.");
            continue;
        }
        if (not versionEntry.section()) {
            logger.warn("Item \"" + name + "\" version not a section.");
            continue;
        }

        Utils::Version version{*versionEntry->label};
        if (version.err) {
            string errMsg{"Item \""};
            errMsg += name;
            errMsg += "\" version \"";
            errMsg += *versionEntry->label;
            errMsg += "\" invalid version str: " + static_cast<string>(version);
            logger.warn(errMsg);
            continue;
        }

        auto hashedVersionEntries{PConf::hash(versionEntry.section()->entries)};

        auto versionHashEntry{hashedVersionEntries.find(HASH_KEY)};
        if (not versionHashEntry) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version "; 
            errMsg += static_cast<string>(version);
            errMsg += " does not have hash for this OS, skipping.";
            logger.info(errMsg);
            continue;
        }
        if (not versionHashEntry->value) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += static_cast<string>(version);
            errMsg += " hash missing value.";
            logger.warn(errMsg);
            continue;
        }
        if (versionHashEntry->value->length() != 64) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += static_cast<string>(version);
            errMsg += " has invalid hash.";
            logger.warn(errMsg);
            continue;
        }

        Update::ItemVersionData versionData;
        versionData.hash = *versionHashEntry->value;

        auto fixEntries{hashedVersionEntries.findAll("FIX")};
        for (const auto& fixEntry : fixEntries) {
            if (not fixEntry->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version "; 
                errMsg += static_cast<string>(version);
                errMsg += " fix missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.fixes.push_back(*fixEntry->value);
        }

        auto changeEntries{hashedVersionEntries.findAll("CHANGE")};
        for (const auto& changeEntry : changeEntries) {
            if (not changeEntry->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += static_cast<string>(version);
                errMsg += " change missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.changes.push_back(*changeEntry->value);
        }

        auto featureEntries{hashedVersionEntries.findAll("FEAT")};
        for (const auto& featureEntry : featureEntries) {
            if (not featureEntry->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += static_cast<string>(version);
                errMsg += " feature missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.features.push_back(*featureEntry->value);
        }

        item.versions.emplace(version, versionData);
    }
     
    return std::pair{ name, item };
}

std::map<Utils::Version, Update::Bundle> resolveBundles(const PConf::HashedData& hashedRawData, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::resolveBundles()")};

    std::map<Utils::Version, Update::Bundle> ret;

    auto bundleEntries{hashedRawData.findAll("BUNDLE")};
    for (const auto& bundleEntry : bundleEntries) {
        if (not bundleEntry->label) {
            logger.warn("Bundle unlabeled.");
            continue;
        }
        if (not bundleEntry.section()) {
            logger.warn("Bundle \"" + bundleEntry->label.value() + "\" not a section.");
            continue;
        }

        Utils::Version version{*bundleEntry->label};
        if (version.err) {
            logger.warn("Bundle \"" + *bundleEntry->label + "\" version invalid: " + static_cast<string>(version));
            continue;
        }

        auto hashedEntries{PConf::hash(bundleEntry.section()->entries)};
        Update::Bundle bundle;

        auto noteEntry{hashedEntries.find("NOTE")};
        if (noteEntry) {
            if (not noteEntry->value) logger.warn("Bundle \"" + static_cast<string>(version) + "\" note missing value");
            else bundle.note = noteEntry->value.value();
        }

        auto parseReqItem{[&logger](const std::shared_ptr<PConf::Entry>& item) -> optional<std::pair<string, Utils::Version>> {
            if (not item->label) {
                logger.warn("Item is unlabeled");
                return nullopt;
            }
            if (not item->value) {
                logger.warn("Item \"" + item->label.value() + "\" is unversioned.");
                return nullopt;
            }
            Utils::Version version{*item->value};
            if (not version) {
                logger.warn("Item \"" + item->label.value() + "\" version \"" + item->value.value() + "\" is invalid: " + static_cast<string>(version));
                return nullopt;
            }

            return std::pair{ *item->label, version };
        }};

        auto fillReqFiles{[&](const vector<PConf::HashedData::IndexedEntryPtr>& entries, Update::ItemType type) {
            for (const auto& entry : entries) {
                auto parsed{parseReqItem(entry)};
                if (parsed) {
                    logger.debug("Added to Bundle " + static_cast<string>(version) + ": " + parsed->first + ", " + static_cast<string>(parsed->second));
                    bundle.reqs.emplace_back(Update::ItemID{ .type=type, .name=parsed->first }, parsed->second);
                }
            }
        }};

        fillReqFiles(hashedEntries.findAll("EXEC"), Update::ItemType::EXEC);
        fillReqFiles(hashedEntries.findAll("LIB"), Update::ItemType::LIB);
        fillReqFiles(hashedEntries.findAll("COMP"), Update::ItemType::COMP);
        fillReqFiles(hashedEntries.findAll("RSRC"), Update::ItemType::RSRC);

        logger.info("Parsed bundle " + static_cast<string>(version));
        ret.emplace(version, bundle);
    }

    return ret;
}

void verifyBundles(const std::map<Update::ItemID, Update::Item>& items, std::map<Utils::Version, Update::Bundle>& bundles, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::verifyBundles()")};

    for (auto bundleIt{bundles.begin()}; bundleIt != bundles.end();) {
        auto& [ version, bundle ]{*bundleIt};
        bool eraseBundle{false};
        for (auto& [ fileID, fileVer, hash] : bundle.reqs) {
            auto itemIt{items.find(fileID)};
            if (itemIt == items.end()) {
                string message{"Bundle "};
                message += static_cast<string>(version);
                message += " contains an item (";
                message += fileID.name + ':' + static_cast<string>(fileVer);
                message += ") which is not registered for this OS, ignoring the item.";
                logger.debug(message);
                fileID.ignored = true;
                continue;
            }

            auto itemVerIt{itemIt->second.versions.find(fileVer)};
            if (itemVerIt == itemIt->second.versions.end()) {
                string message{"Bundle "};
                message += static_cast<string>(version);
                message += " contains a version of an item (";
                message += fileID.name + ':' + static_cast<string>(fileVer);
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

