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

#include <cstdlib>
#include <thread>
#include <fstream>

#include <wx/webrequest.h>

#include <utils/paths.h>
#include <pconf/pconf.h>
#include <log/logger.h>

#include "update.h"
#include "wx/utils.h"

namespace Update {

/**
 * @return True if a fatal message was found.
 */
[[nodiscard]] bool checkMessages(const PConf::HashedData&, Log::Branch&);

[[nodiscard]] std::map<ItemID, Item> findItems(const PConf::HashedData&, Log::Branch&);

[[nodiscard]] optional<std::pair<string, Item>> parseItem(const std::shared_ptr<PConf::Entry>&, Log::Logger&);

[[nodiscard]] std::map<Version, Bundle> resolveBundles(const PConf::HashedData&, Log::Branch&);

void verifyBundles(const std::map<ItemID, Item>& items, std::map<Version, Bundle>& bundles, Log::Branch&);

} // namespace Update

bool Update::pullData(PCUI::ProgressDialog *prog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::pullData()")};

    bool requestComplete{false};
    auto handleRequestEvent{[&](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxCopyFile(evt.GetDataFile(), manifestFile().string());
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

    const auto pullFrom{Paths::remoteUpdateAssets() + "/manifest.pconf"};
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
        logger.warn("Data pull failed.");
        return false;
    }

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

    return Data{ std::move(items), std::move(bundles) };
}

bool Update::checkMessages(const PConf::HashedData& hashedRawData, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::checkMessages()")};

    Version launcherVersion{wxSTRINGIZE(EXEC_VERSION)};
    if (not launcherVersion) {
        logger.error("This launcher's version is invalid (" + string(launcherVersion) + "), messages will not be available!!");
        return false;
    }

    bool hadFatal{false};
    auto messageRange{hashedRawData.equal_range("MESSAGE")};
    for (auto messageIt{messageRange.first}; messageIt != messageRange.second; ++messageIt) {
        if (messageIt->second->getType() != PConf::Type::SECTION) {
            logger.warn("Message entry found in pconf! (Not a section)");
            continue;
        }

        auto msgSect{std::static_pointer_cast<PConf::Section>(messageIt->second)};
        if (not msgSect->label) {
            logger.warn("Message without version specified is unacceptable.");
            continue;
        }

        auto hashedEntries{PConf::hash(msgSect->entries)};
        auto content{hashedEntries.find("CONTENT")};
        if (content == hashedEntries.end()) {
            logger.warn("Message missing content!");
            continue;
        }
        if (not content->second->value) {
            logger.warn("Message content entry missing value!");
            continue;
        }
        bool isFatal{hashedEntries.find("FATAL") != hashedEntries.end()};

        auto label{msgSect->label.value()};
        Comparator comp{};
        if (label.length() > 1) {
            if (label[0] == '<') comp = Comparator::LESS_THAN;
            else if (label[0] == '>') comp = Comparator::GREATER_THAN;
            else if (label[0] == '=' or std::isdigit(label[0])) comp = Comparator::EQUAL;
            else {
                logger.warn("Message version invalid (comparator).");
                continue;
            }
        }
        if (not std::isdigit(label[0])) label = label.substr(1);
        Version version{label};
        if (not version) {
            logger.warn("Failed to parse message version: " + string(version));
            continue;
        }

        bool messageApplicable{};
        switch (comp) {
            case Comparator::EQUAL:
                messageApplicable = launcherVersion == version;
                break;
            case Comparator::LESS_THAN:
                messageApplicable = launcherVersion < version;
                break;
            case Comparator::GREATER_THAN:
                messageApplicable = launcherVersion > version;
                break;
        }
        if (messageApplicable) {
            PCUI::showMessage(content->second->value.value(), isFatal ? "ProffieConfig Launcher Critical Notice" : "ProffieConfig Launcher Alert");
            if (isFatal) hadFatal = true;
        }
    }

    return hadFatal;
}

std::map<Update::ItemID, Update::Item> Update::findItems(const PConf::HashedData& rawHashedData, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::enumerateItems()")};

    std::map<ItemID, Item> ret;

    auto findType{[&logger, &ret](std::pair<PConf::HashedData::const_iterator, PConf::HashedData::const_iterator> range, ItemType type, const string& typeStr) {
        for (auto itemIt{range.first}; itemIt != range.second; ++itemIt) {
            auto parsed{parseItem(itemIt->second, logger)};
            if (parsed) {
                logger.info("Sucessfully parsed " + typeStr + ' ' + parsed->first);
                ret.emplace(ItemID{ type, parsed->first}, parsed->second);
            }
        }
    }};

    findType(rawHashedData.equal_range("EXEC"), ItemType::EXEC, "executable");
    findType(rawHashedData.equal_range("LIB"), ItemType::LIB, "library");
    findType(rawHashedData.equal_range("COMP"), ItemType::COMP, "component");
    findType(rawHashedData.equal_range("RSRC"), ItemType::RSRC, "resource");

    return ret;
}

optional<std::pair<string, Update::Item>> Update::parseItem(const std::shared_ptr<PConf::Entry>& entry, Log::Logger& logger) {
    if (not entry->label) {
        logger.warn("Item missing name!");
        return nullopt; 
    }
    auto name{entry->label.value()};

    if (entry->getType() != PConf::Type::SECTION) {
        logger.warn("Item \"" + name + "\" not a section!");
        return nullopt;
    }
    auto sect{std::static_pointer_cast<PConf::Section>(entry)};

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

    auto hashedEntries{PConf::hash(sect->entries)};
    auto path{hashedEntries.find(PATH_KEY)};
    if (path == hashedEntries.end()) {
        logger.info("Item \"" + name + "\" missing platform path, ignoring.");
        return nullopt;
    }
    if (not path->second->value) {
        logger.warn("Item \"" + name + "\" path missing value!");
        return nullopt;
    }

    Item item;
    item.hidden = hashedEntries.find("HIDDEN") != hashedEntries.end();
    item.path = path->second->value.value();

    auto versionRange{hashedEntries.equal_range("VERSION")};
    for (auto versionIt{versionRange.first}; versionIt != versionRange.second; ++versionIt) {
        if (not versionIt->second->label) {
            logger.warn("Item \"" + name + "\" version unlabeled.");
            continue;
        }
        if (versionIt->second->getType() != PConf::Type::SECTION) {
            logger.warn("Item \"" + name + "\" version not a section.");
            continue;
        }

        Version version{versionIt->second->label.value()};
        if (not version) {
            string errMsg{"Item \""};
            errMsg += name;
            errMsg += "\" version \"";
            errMsg += versionIt->second->label.value(); 
            errMsg += "\" invalid version str: " + string(version);
            logger.warn(errMsg);
            continue;
        }

        auto versionSect{std::static_pointer_cast<PConf::Section>(versionIt->second)};
        auto hashedVersionEntries{PConf::hash(versionSect->entries)};

        auto versionHash{hashedVersionEntries.find(HASH_KEY)};
        if (versionHash == hashedVersionEntries.end()) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version "; 
            errMsg += string{version};
            errMsg += " does not have hash for this OS, skipping.";
            logger.info(errMsg);
            continue;
        }
        if (not versionHash->second->value) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += string(version);
            errMsg += " hash missing value.";
            logger.warn(errMsg);
            continue;
        }
        if (versionHash->second->value->length() != 64) {
            string errMsg{"Item \""}; 
            errMsg += name;
            errMsg += "\" version ";
            errMsg += string(version);
            errMsg += " has invalid hash.";
            logger.warn(errMsg);
            continue;
        }

        ItemVersionData versionData;
        versionData.hash = versionHash->second->value.value();

        auto fixes{hashedVersionEntries.equal_range("FIX")};
        for (auto fixIt{fixes.first}; fixIt != fixes.second; ++fixIt) {
            if (not fixIt->second->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version "; 
                errMsg += string(version);
                errMsg += " fix missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.fixes.push_back(fixIt->second->value.value());
        }

        auto changes{hashedVersionEntries.equal_range("CHANGE")};
        for (auto changeIt{changes.first}; changeIt != changes.second; ++changeIt) {
            if (not changeIt->second->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += " change missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.changes.push_back(changeIt->second->value.value());
        }

        auto features{hashedVersionEntries.equal_range("FEAT")};
        for (auto featIt{features.first}; featIt != features.second; ++featIt) {
            if (not featIt->second->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += " feature missing value.";
                logger.warn(errMsg);
                continue;
            }
            versionData.features.push_back(featIt->second->value.value());
        }

        item.versions.emplace(version, versionData);
    }
     
    return std::pair{ name, item };
}

std::map<Update::Version, Update::Bundle> Update::resolveBundles(const PConf::HashedData& hashedRawData, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::resolveBundles()")};

    std::map<Version, Bundle> ret;

    auto bundleRange{hashedRawData.equal_range("BUNDLE")};
    for (auto bundleIt{bundleRange.first}; bundleIt != bundleRange.second; ++bundleIt) {
        if (not bundleIt->second->label) {
            logger.warn("Bundle unlabeled.");
            continue;
        }
        if (bundleIt->second->getType() != PConf::Type::SECTION) {
            logger.warn("Bundle \"" + bundleIt->second->label.value() + "\" not a section.");
            continue;
        }

        Version version{bundleIt->second->label.value()};
        if (not version) {
            logger.warn("Bundle \"" + bundleIt->second->label.value() + "\" version invalid: " + string(version));
            continue;
        }

        auto hashedEntries{PConf::hash(std::static_pointer_cast<PConf::Section>(bundleIt->second)->entries)};
        Bundle bundle;

        auto noteIt{hashedEntries.find("NOTE")};
        if (noteIt != hashedEntries.end()) {
            if (not noteIt->second->value) logger.warn("Bundle \"" + string(version) + "\" note missing value");
            else bundle.note = noteIt->second->value.value();
        }

        auto parseReqItem{[&logger](const std::shared_ptr<PConf::Entry>& item) -> optional<std::pair<string, Version>> {
            if (not item->label) {
                logger.warn("Item is unlabeled");
                return nullopt;
            }
            if (not item->value) {
                logger.warn("Item \"" + item->label.value() + "\" is unversioned.");
                return nullopt;
            }
            Version version{item->value.value()};
            if (not version) {
                logger.warn("Item \"" + item->label.value() + "\" version \"" + item->value.value() + "\" is invalid: " + static_cast<string>(version));
                return nullopt;
            }

            return std::pair{ item->label.value(), version };
        }};

        auto fillReqFiles{[&](std::pair<PConf::HashedData::const_iterator, PConf::HashedData::const_iterator> range, ItemType type) {
            for (auto itemIt{range.first}; itemIt != range.second; ++itemIt) {
                auto parsed{parseReqItem(itemIt->second)};
                if (parsed) {
                    logger.debug("Added to Bundle " + static_cast<string>(version) + ": " + parsed->first + ", " + static_cast<string>(parsed->second));
                    bundle.reqs.emplace_back(ItemID{ type, parsed->first }, parsed->second);
                }
            }
        }};

        fillReqFiles(hashedEntries.equal_range("EXEC"), ItemType::EXEC);
        fillReqFiles(hashedEntries.equal_range("LIB"), ItemType::LIB);
        fillReqFiles(hashedEntries.equal_range("COMP"), ItemType::COMP);
        fillReqFiles(hashedEntries.equal_range("RSRC"), ItemType::RSRC);

        logger.info("Parsed bundle " + static_cast<string>(version));
        ret.emplace(version, bundle);
    }

    return ret;
}

void Update::verifyBundles(const std::map<ItemID, Item>& items, std::map<Version, Bundle>& bundles, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::verifyBundles()")};

    for (auto bundleIt{bundles.begin()}; bundleIt != bundles.end();) {
        auto& [ version, bundle ]{*bundleIt};
        bool eraseBundle{false};
        for (auto& [ fileID, fileVer, hash] : bundle.reqs) {
            auto itemIt{items.find(fileID)};
            if (itemIt == items.end()) {
                logger.warn("Bundle " + static_cast<string>(version) + " invalid (req file \"" + fileID.name + "\" unregistered)");
                eraseBundle = true;
                break;
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

