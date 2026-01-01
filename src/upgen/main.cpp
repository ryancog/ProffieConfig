/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * upgen/main.cpp
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

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <set>
#include <map>

#include <wx/app.h>

#include "log/context.h"
#include "log/severity.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/crypto.h"
#include "utils/paths.h"
#include "utils/types.h"

#include "../launcher/update/update.h"

class UpGen : public wxAppConsole {
private:
    enum Platform {
        MACOS,
        WINDOWS, // Not WIN32 because some stupid windows define...
        LINUX,
        PLATFORM_MAX
    };

    static constexpr cstring HEADER{
        "+--------------------------------------------------+\n"
        "|  KT Update Manifest Generator                    |\n"
        "+--------------------------------------------------+\n"
    };

    static constexpr cstring STAGING_DIR{"./staging"};
    static constexpr cstring MANIFEST_DEFAULT{"manifest.pconf"};

    static constexpr cstring PATH_KEY_WIN32{"PATH_Win32"};
    static constexpr cstring PATH_KEY_MACOS{"PATH_macOS"};
    static constexpr cstring PATH_KEY_LINUX{"PATH_Linux"};
    static constexpr cstring HASH_KEY_WIN32{"HASH_Win32"};
    static constexpr cstring HASH_KEY_MACOS{"HASH_macOS"};
    static constexpr cstring HASH_KEY_LINUX{"HASH_Linux"};

    static constexpr std::array<cstring, PLATFORM_MAX> PLATFORM_DIRS {
        "macOS",
        "win32",
        "linux",
    };

    using ItemMap = std::map<filepath, string>;
    using FileMaps = std::array<std::array<ItemMap, Update::TYPE_MAX>, PLATFORM_MAX>;

    struct Message {
        Utils::Version version;
        Update::Comparator versionComp;
        string message;
        bool fatal;
    };
    struct ItemVersionData {
        string macOSHash;
        string win32Hash;
        string linuxHash;

        vector<string> fixes;
        vector<string> changes;
        vector<string> features;
    };

    struct Item {
        optional<filepath> macOSPath;
        optional<filepath> win32Path;
        optional<filepath> linuxPath;

        std::map<Utils::Version, ItemVersionData> versions;
        bool hidden;
        bool deprecated;
    };

    using Items = std::map<Update::ItemID, Item>;

    struct Data {
        Items items;
        Update::Bundles bundles;
    };


    static std::ostream& resetLine() { std::cout << "\33[2K\r"; return std::cout; }
    static std::ostream& resetToPrevLine() { 
        std::cout << "\033[A";
        return resetLine();
    }
    static void updatePrevLine(const string& str) { 
        resetToPrevLine() << str << '\n' << std::flush;
    }
    static void clearScreen() {
        std::cout << "\033[2J\033[H" << std::flush;
    }

    static void checkDir(const filepath& folder) {
        std::error_code err;
        if (not fs::is_directory(folder, err)) {
            std::wcout << "Warn: creating `" << fs::relative(folder, STAGING_DIR).wstring() << "`\n";
            fs::create_directories(folder);
        }
    }

    static std::pair<Data, vector<Message>> loadCurrentManifest(const filepath&);

    static vector<Message> parseMessages(const PConf::HashedData&, Log::Logger&);
    static std::pair<string, Item> parseItem(const PConf::EntryPtr&, Log::Logger&);
    static std::pair<string, Item> parseBundles(const PConf::EntryPtr&, Log::Logger&);
    static Update::Bundles resolveBundles(const PConf::HashedData&, Log::Logger&);
    static void verifyBundles(const Items&, const Update::Bundles&, Log::Logger&);

    static void handleNewItems(Data&, FileMaps);
    static void addNotesToItemVersion(const string& itemName, const Item&, ItemVersionData&);

    static void generateNewManifest(const vector<Message>&, const Data&);
    static void organizeAssets(const FileMaps&, const Data&);

    static cstring itemTypeToStr(Update::ItemType);

    static void listMessages(const vector<Message>&, bool hold = true);
    static void addMessage(vector<Message>&);
    static void removeMessage(vector<Message>&);
    static void listItems(const Items&, bool hold = true);
    static void toggleItemVisibility(Items&);
    static void toggleItemDeprecation(Items&);
    static void listBundles(const Update::Bundles&, bool hold = true);
    static void addBundle(Data&, bool fromCurrent);
    static void removeBundle(Update::Bundles&);
    static void genBundleChangelog(Data&);

public:
    bool OnInit() override {
        Log::Context::setGlobalOuput({&std::clog}, false);
        auto& globalContext{Log::Context::getGlobal()};
        globalContext.setSeverity(Log::Severity::ERR);
        auto& logger{globalContext.createLogger("main()")};

        clearScreen();
        std::cout << HEADER << '\n' << std::flush;

        FileMaps fileMaps{};

        std::iostream::sync_with_stdio();
        std::cout << "Checking directories...\n";
        checkDir(STAGING_DIR);
        for (auto platformIdx{0}; platformIdx < PLATFORM_MAX; ++platformIdx) {
            auto platformDir{filepath{STAGING_DIR} / PLATFORM_DIRS[platformIdx]};
            checkDir(platformDir);
            for (auto typeIdx{0}; typeIdx < Update::TYPE_MAX; ++typeIdx) {
                checkDir(platformDir / Update::typeFolder(static_cast<Update::ItemType>(typeIdx)));
            }
        }

        std::cout << "Finding files in `staging` directory...\n";
        for (auto platformIdx{0}; platformIdx < PLATFORM_MAX; ++platformIdx) {
            const auto platformFolder{filepath{STAGING_DIR} / PLATFORM_DIRS[platformIdx]};
            for (auto typeIdx{0}; typeIdx < Update::TYPE_MAX; ++typeIdx) {
                const auto dirFolder{platformFolder / Update::typeFolder(static_cast<Update::ItemType>(typeIdx))};
                for (const auto& entry : fs::recursive_directory_iterator{dirFolder}) {
                    if (entry.is_directory()) continue;
                    resetLine() << "Checking file " << entry << "... " << std::flush;
                    if (not entry.is_regular_file()) {
                        std::cout << "Not a regular file, skipping.\n";
                        continue;
                    }
                    if (entry.path().filename() == ".DS_Store") {
                        std::cout << "Ignoring metadata file.\n";
                        continue;
                    }

                    std::ifstream stream{entry.path()};
                    auto fileHash{Crypto::Hash::stream(stream)};
                    fileMaps[platformIdx][typeIdx].emplace(
                        fs::relative(entry.path(), dirFolder),
                        fileHash
                    );
                }
            }
        }

        resetLine().flush();
        constexpr cstring MANIFEST_PROMPT{"Manifest file"};
        std::cout << MANIFEST_PROMPT << " [" << MANIFEST_DEFAULT << "]: " << std::flush;
        string manifestStr;
        std::getline(std::cin, manifestStr);
        if (manifestStr.empty()) {
            updatePrevLine(string(MANIFEST_PROMPT) + " [" + MANIFEST_DEFAULT + "]: " + MANIFEST_DEFAULT);
            manifestStr = MANIFEST_DEFAULT;
        }
        filepath manifestFile{manifestStr};
        if (manifestFile.is_relative()) manifestFile = STAGING_DIR / manifestFile;

        std::error_code err;
        if (not fs::exists(manifestFile, err)) {
            std::cout << "Manifest file does not exist!\n";
            exit(1);
        }
        if (not fs::is_regular_file(manifestFile)) {
            std::cout << "Manifest file is not a file!\n";
            exit(1);
        }

        std::cout << "Parsing manifest... " << std::flush;
        auto [ data, messages ]{loadCurrentManifest(manifestFile)};
        std::cout << "Done.\n" << std::flush;

        std::cout << "Running through files...\n" << std::flush;
        handleNewItems(data, fileMaps);

        string input;
        std::cout << "Files updated. [ENTER] " << std::flush;
        std::getline(std::cin, input);

        while (not false) {
            clearScreen();
            std::cout << 
                HEADER <<
                "|  0) Write manifest, organize assets and exit     |\n"
                "|  1) List all messages                            |\n"
                "|  2) Add Message                                  |\n"
                "|  3) Remove Message                               |\n"
                "|  4) List all items                               |\n"
                "|  5) Toggle item visibility                       |\n"
                "|  6) Toggle item deprecation                      |\n"
                "|  7) List all bundles                             |\n"
                "|  8) Add new bundle (Fresh)                       |\n"
                "|  9) Add new bundle (From Current Items)          |\n"
                "|  10) Remove bundle                               |\n"
                "|  11) Generate changelog for bundle               |\n"
                "+--------------------------------------------------+\n"
                "Selection: " << std::flush;
            std::getline(std::cin, input);

            if (input == "0") break;

            if (input == "1") listMessages(messages);
            else if (input == "2") addMessage(messages);
            else if (input == "3") removeMessage(messages);
            else if (input == "4") listItems(data.items);
            else if (input == "5") toggleItemVisibility(data.items);
            else if (input == "6") toggleItemDeprecation(data.items);
            else if (input == "7") listBundles(data.bundles);
            else if (input == "8") addBundle(data, false);
            else if (input == "9") addBundle(data, true);
            else if (input == "10") removeBundle(data.bundles);
            else if (input == "11") genBundleChangelog(data);
            else {
                std::cout << "Invalid input. " << std::flush;
                std::getline(std::cin, input);
                continue;
            }
        }           

        std::cout << "Generating new manifest... " << std::flush;
        auto oldLocation{filepath{STAGING_DIR} / (string{MANIFEST_DEFAULT} + ".old")};
        std::filesystem::remove(oldLocation);
        std::filesystem::copy_file(manifestFile, oldLocation);
        generateNewManifest(messages, data);
        std::cout << "Done.\n" << std::flush;
        std::cout << "Organizing available assets... " << std::flush;
        organizeAssets(fileMaps, data);
        std::cout << "Done.\n" << std::flush;

        return false;
    }
};

wxIMPLEMENT_APP_CONSOLE(UpGen);

std::pair<UpGen::Data, vector<UpGen::Message>> UpGen::loadCurrentManifest(const filepath& manifest) {
    auto& logger{Log::Context::getGlobal().createLogger("loadCurrentManifest()")};

    auto manifestStream{Paths::openInputFile(manifest)};
    PConf::Data rawData;
    if (not PConf::read(manifestStream, rawData, logger.binfo("Reading manifest..."))) {
        exit(1);
    }

    logger.info("Parsing manifest...");
    auto hashedRawData{PConf::hash(rawData)};

    vector<Message> messages{parseMessages(hashedRawData, logger)};
    UpGen::Data data;

    auto enumerateType{[&](
        const vector<PConf::EntryPtr>& vec, Update::ItemType type
    ) {
        for (const auto& item : vec) {
            auto parsed{parseItem(item, logger)};
            data.items.emplace(
                Update::ItemID{.type=type, .name=parsed.first}, parsed.second
            );
        }
    }};
    enumerateType(hashedRawData.findAll("EXEC"), Update::ItemType::EXEC);
    enumerateType(hashedRawData.findAll("LIB"),  Update::ItemType::LIB);
    enumerateType(hashedRawData.findAll("COMP"), Update::ItemType::COMP);
    enumerateType(hashedRawData.findAll("RSRC"), Update::ItemType::RSRC);

    data.bundles = resolveBundles(hashedRawData, logger);
    verifyBundles(data.items, data.bundles, logger);

    return { data, messages };
}

vector<UpGen::Message> UpGen::parseMessages(const PConf::HashedData& hashedRawData, Log::Logger& logger) {
    vector<UpGen::Message> ret;
    auto messageEntries{hashedRawData.findAll("MESSAGE")};
    for (const auto& messageEntry : messageEntries) {
        if (not messageEntry.section()) {
            logger.error("Message entry found in pconf! (Not a section)");
            exit(1);
        }

        if (not messageEntry->label) {
            logger.error("Message without version specified is unacceptable.");
            exit(1);
        }

        auto hashedEntries{PConf::hash(messageEntry.section()->entries)};
        auto contentEntry{hashedEntries.find("CONTENT")};
        if (not contentEntry) {
            logger.error("Message missing content.");
            exit(1);
        }

        if (not contentEntry->value) {
            logger.error("Message content entry missing value.");
            exit(1);
        }

        auto label{*messageEntry->label};
        Update::Comparator comp{};
        if (label.length() > 1) {
            if (label[0] == '<') comp = Update::Comparator::LESS_THAN;
            else if (label[0] == '>') comp = Update::Comparator::GREATER_THAN;
            else if (label[0] == '=' or std::isdigit(label[0])) comp = Update::Comparator::EQUAL;
            else {
                logger.error("Message version invalid (comparator).");
                exit(1);
            }
        }
        if (not std::isdigit(label[0])) label = label.substr(1);
        Utils::Version version{label};
        if (not version) {
            logger.error("Failed to parse message version: " + string(version));
            exit(1);
        }

        auto& message{ret.emplace_back()};
        message.fatal = static_cast<bool>(hashedEntries.find("FATAL"));
        message.version = version;
        message.versionComp = comp;
        message.message = *contentEntry->value;
    }
    return ret;
}

std::pair<string, UpGen::Item> UpGen::parseItem(const PConf::EntryPtr& entry, Log::Logger& logger) {
    if (not entry->label) {
        logger.error("Item missing name!");
        exit(1);
    }
    auto name{*entry->label};

    if (not entry.section()) {
        logger.error("Item \"" + name + "\" not a section!");
        exit(1);
    }

    auto hashedEntries{PConf::hash(entry.section()->entries)};
    auto checkPath{[&](cstring spec, cstring key) -> optional<filepath> {
        auto pathEntry{hashedEntries.find(key)};

        // It may not have a path for certain things, that's fine.
        if (not pathEntry) return nullopt;

        if (not pathEntry->value) {
            logger.error("Item \"" + name + "\" " + spec + " path missing value!");
            exit(1);
        }
        if (pathEntry->value->empty()) {
            logger.error("Item \"" + name + "\" has empty " + spec + " path!");
            exit(1);
        }
        return pathEntry->value;
    }};

    Item item;
    item.linuxPath = checkPath("Linux", PATH_KEY_LINUX);
    item.win32Path = checkPath("Win32", PATH_KEY_WIN32);
    item.macOSPath = checkPath("macOS", PATH_KEY_MACOS);
    item.hidden = static_cast<bool>(hashedEntries.find("HIDDEN"));
    item.deprecated = static_cast<bool>(hashedEntries.find("DEPRECATED"));

    auto versionEntries{hashedEntries.findAll("VERSION")};
    for (const auto& versionEntry : versionEntries) {
        if (not versionEntry->label) {
            logger.error("Item \"" + name + "\" version unlabeled.");
            exit(1);
        }
        if (not versionEntry.section()) {
            logger.error("Item \"" + name + "\" version not a section.");
            exit(1);
        }

        Utils::Version version{*versionEntry->label};
        if (not version) {
            string errMsg{"Item \""};
            errMsg += name;
            errMsg += "\" version \"";
            errMsg += *versionEntry->label;
            errMsg += "\" invalid version str: " + string(version);
            logger.error(errMsg);
            exit(1);
        }

        auto hashedVersionEntries{PConf::hash(versionEntry.section()->entries)};

        auto checkHash{[&](cstring spec, cstring key) {
            auto versionHashEntry{hashedVersionEntries.find(key)};
            if (not versionHashEntry) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version "; 
                errMsg += string(version);
                errMsg += string(" missing ") + spec + " hash.";
                logger.error(errMsg);
                exit(1);
            }
            if (not versionHashEntry->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += string(" ") + spec + " hash missing value.";
                logger.error(errMsg);
                exit(1);
            }
            if (versionHashEntry->value->length() != 64) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += string(" has invalid ") + spec + " hash.";
                logger.error(errMsg);
                exit(1);
            }
            return versionHashEntry->value.value();
        }};

        ItemVersionData versionData;
        if (item.linuxPath) versionData.linuxHash = checkHash("Linux", HASH_KEY_LINUX);
        if (item.win32Path) versionData.win32Hash = checkHash("Win32", HASH_KEY_WIN32);
        if (item.macOSPath) versionData.macOSHash = checkHash("macOS", HASH_KEY_MACOS);

        auto fixes{hashedVersionEntries.findAll("FIX")};
        for (const auto& fix : fixes) {
            if (not fix->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version "; 
                errMsg += string(version);
                errMsg += " fix missing value.";
                logger.error(errMsg);
                exit(1);
            }
            versionData.fixes.push_back(fix->value.value());
        }

        auto changes{hashedVersionEntries.findAll("CHANGE")};
        for (const auto& change : changes) {
            if (not change->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += " change missing value.";
                logger.error(errMsg);
                exit(1);
            }
            versionData.changes.push_back(change->value.value());
        }

        auto features{hashedVersionEntries.findAll("FEAT")};
        for (const auto& feat : features) {
            if (not feat->value) {
                string errMsg{"Item \""}; 
                errMsg += name;
                errMsg += "\" version ";
                errMsg += string(version);
                errMsg += " feature missing value.";
                logger.error(errMsg);
                exit(1);
            }
            versionData.features.push_back(feat->value.value());
        }

        item.versions.emplace(version, versionData);
    }
    if (versionEntries.empty()) {
        logger.error("Item \"" + name + "\" has no versions!");
        exit(1);
    }
     
    return { name, item };
}

Update::Bundles UpGen::resolveBundles(const PConf::HashedData& hashedRawData, Log::Logger& logger) {
    Update::Bundles ret;

    auto bundleEntries{hashedRawData.findAll("BUNDLE")};
    for (const auto& bundleEntry : bundleEntries) {
        if (not bundleEntry->label) {
            logger.error("Bundle unlabeled.");
            exit(1);
        }
        if (not bundleEntry.section()) {
            logger.error("Bundle \"" + *bundleEntry->label + "\" not a section.");
            exit(1);
        }

        Utils::Version version{*bundleEntry->label};
        if (not version) {
            logger.error("Bundle \"" + *bundleEntry->label + "\" version invalid: " + string(version));
            exit(1);
        }

        auto hashedEntries{PConf::hash(bundleEntry.section()->entries)};
        Update::Bundle bundle;

        auto noteEntry{hashedEntries.find("NOTE")};
        if (noteEntry) {
            if (not noteEntry->value) {
                logger.error("Bundle \"" + static_cast<string>(version) + "\" note missing value");
                exit(1);
            }

            bundle.note = *noteEntry->value;
        }

        const auto parseReqItem{[&logger, version](const PConf::EntryPtr& item) -> optional<std::pair<string, Utils::Version>> {
            if (not item->label) {
                logger.error("Item is unlabeled");
                exit(1);
            }
            if (not item->value) {
                logger.error("Item \"" + item->label.value() + "\" is unversioned.");
                exit(1);
            }
            Utils::Version version{*item->value};
            if (not version) {
                logger.error("Item \"" + item->label.value() + "\" version \"" + item->value.value() + "\" is invalid: " + string(version));
                exit(1);
            }

            return std::pair{ item->label.value(), version };
        }};

        const auto fillReqFiles{[&](
            const vector<PConf::EntryPtr>& vec, Update::ItemType type
        ) {
            bundle.reqs.reserve(vec.size());
            for (const auto& entry : vec) {
                auto parsed{parseReqItem(entry)};
                if (not parsed) continue;

                bundle.reqs.emplace_back(
                    Update::ItemID{.type=type, .name=parsed->first},
                    parsed->second
                );
            }
        }};

        fillReqFiles(hashedEntries.findAll("EXEC"), Update::ItemType::EXEC);
        fillReqFiles(hashedEntries.findAll("LIB"),  Update::ItemType::LIB);
        fillReqFiles(hashedEntries.findAll("COMP"), Update::ItemType::COMP);
        fillReqFiles(hashedEntries.findAll("RSRC"), Update::ItemType::RSRC);
        ret.emplace(version, bundle);
    }

    return ret;
}

void UpGen::verifyBundles(const Items& items, const Update::Bundles& bundles, Log::Logger& logger) {
    for (const auto& [ version, bundle ] : bundles) {
        bool eraseBundle{false};
        for (const auto& [ fileID, fileVer, hash] : bundle.reqs) {
            auto itemIt{items.find(fileID)};
            if (itemIt == items.end()) {
                logger.error("Bundle " + string(version) + " invalid (req file \"" + fileID.name + "\" unregistered)");
                exit(1);
            }
            auto itemVerIt{itemIt->second.versions.find(fileVer)};
            if (itemVerIt == itemIt->second.versions.end()) {
                logger.error("Bundle " + string(version) + " invalid (req file \"" + fileID.name + "\" invalid version \"" + string(fileVer)  + "\")");
                exit(1);
            }
        }
    }
}

// maps must be a copy!!
void UpGen::handleNewItems(Data& data, FileMaps fileMaps) {
    // Clear out items we already know about.
    for (auto& [ itemID, item ] : data.items) {
        auto getFileHash{[&](Platform idx, const optional<filepath>& path) {
            auto mapEnd{fileMaps[idx][itemID.type].end()};
            if (not path) return std::pair{ optional<string>{nullopt}, mapEnd };

            auto fileIt{fileMaps[idx][itemID.type].find(path.value())};
            if (fileIt == mapEnd) return std::pair { optional<string>{nullopt}, mapEnd };

            return std::pair{ optional<string>{fileIt->second}, fileIt };
        }};
        auto [ win32FileHash, win32It ]{getFileHash(WINDOWS, item.win32Path)};
        auto [ macOSFileHash, macOSIt ]{getFileHash(MACOS, item.macOSPath)};
        auto [ linuxFileHash, linuxIt ]{getFileHash(LINUX, item.linuxPath)};

        bool win32Handled{not win32FileHash};
        bool macOSHandled{not macOSFileHash};
        bool linuxHandled{not linuxFileHash};
        for (const auto& [ version, versionData ] : item.versions) {
            if (not win32Handled and win32FileHash.value() == versionData.win32Hash) win32Handled = true;
            if (not macOSHandled and macOSFileHash.value() == versionData.macOSHash) macOSHandled = true;
            if (not linuxHandled and linuxFileHash.value() == versionData.linuxHash) linuxHandled = true;
            if (win32Handled and macOSHandled and linuxHandled) break;
        }

        if (not win32Handled or not macOSHandled or not linuxHandled) {
            auto currentVer{item.versions.rbegin()->first};
            auto suggestedVer{currentVer};
            ++suggestedVer.bugfix;
            while (not false) {
                string input;
                resetLine() << 
                    "New version for " << itemTypeToStr(itemID.type) << ' ' << itemID.name << 
                    " (current: " << static_cast<string>(currentVer) << ") [" << static_cast<string>(suggestedVer) << "]: " << std::flush;
                std::getline(std::cin, input);
                resetToPrevLine();

                Utils::Version version;

                if (input.empty()) version = suggestedVer;
                else {
                    // Update as if new build is functionally equivalent to last
                    if (input == "s") {
                        version = Utils::Version::invalidObject();
                    } else {
                        version = Utils::Version{input};
                        if (not version) {
                            std::cout << "Invalid version entered, try again. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }

                        if (item.versions.contains(version)) {
                            std::cout << "Version exists, try again. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }
                    }
                }

                if (version != Utils::Version::invalidObject()) {
                    ItemVersionData versionData;
                    versionData.linuxHash = linuxFileHash.value_or("");
                    versionData.macOSHash = macOSFileHash.value_or("");
                    versionData.win32Hash = win32FileHash.value_or("");
                    addNotesToItemVersion(itemID.name, item, versionData);
                    item.versions.emplace(version, versionData);
                }
                break;
            }

            std::cout << "Updated \"" << itemID.name << "\" to version "
                << static_cast<string>(item.versions.rbegin()->first) << '\n' << std::flush;
        }

        if (win32FileHash) fileMaps[WINDOWS][itemID.type].erase(win32It);
        if (macOSFileHash) fileMaps[MACOS][itemID.type].erase(macOSIt);
        if (linuxFileHash) fileMaps[LINUX][itemID.type].erase(linuxIt);
    }

    for (auto platformIdx{0}; platformIdx < PLATFORM_MAX; ++platformIdx) {
        cstring platformStr{PLATFORM_DIRS[Platform(platformIdx)]};
        for (auto typeIdx{0}; typeIdx < Update::TYPE_MAX; ++typeIdx) {
            cstring typeStr{itemTypeToStr(static_cast<Update::ItemType>(typeIdx))};
            auto itemType{static_cast<Update::ItemType>(typeIdx)};
            for (const auto& [ path, hash ] : fileMaps[platformIdx][typeIdx]) {
                string input;
                Item *item{nullptr};
                string itemName;
                bool newVersion{false};
                bool newItem{false};

                while (not false) {
                    if (typeIdx == Update::RSRC) {
                        itemName = path.string();
                    } else {
                        std::cout << "Item to add " << platformStr << ' ' << typeStr << ' ' << path << " to: " << std::flush;
                        std::getline(std::cin, input);
                        resetToPrevLine();

                        if (input.empty()) {
                            std::cout << "Invalid item name, try again. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }

                        itemName = input;
                    }

                    auto itemIt{data.items.find({ itemType, itemName })};
                    if (itemIt == data.items.end()) {
                        if (typeIdx == Update::RSRC) {
                            item = &data.items.emplace(Update::ItemID{ .type=itemType, .name=itemName }, Item{}).first->second;
                            item->hidden = true;
                            newItem = true;
                        } else {
                            while (not false) {
                                std::cout << "Item does not exist, create it? [y/N]: " << std::flush;
                                std::getline(std::cin, input);
                                resetToPrevLine();

                                if (input == "y" or input == "Y") {
                                    item = &data.items.emplace(Update::ItemID{ .type=itemType, .name=itemName }, Item{}).first->second;
                                    newItem = true;
                                } else if (input.empty() or input == "n" or input == "N") break;
                                else continue;

                                while (not false) {
                                    std::cout << "Make item hidden? [y/N]: " << std::flush;
                                    std::getline(std::cin, input);
                                    resetToPrevLine();

                                    if (input.empty() or input == "n" or input == "N") item->hidden = false;
                                    else if (input == "y" or input == "Y") item->hidden = true;
                                    else continue;

                                    break;
                                }

                                break;
                            }

                            if (not item) continue;
                        }
                    } else {
                        optional<filepath> currentPath{};
                        switch (Platform(platformIdx)) {
                            case MACOS: 
                                currentPath = itemIt->second.macOSPath;
                                break;
                            case WINDOWS:
                                currentPath = itemIt->second.win32Path;
                                break;
                            case LINUX:
                                currentPath = itemIt->second.linuxPath;
                                break;
                            case PLATFORM_MAX: break;
                        }
                        if (currentPath and currentPath.value() != path) {
                            std::cout << "This item already has a file for " << PLATFORM_DIRS[platformIdx] << ", choose/create another item. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }
                        item = &itemIt->second;
                    }

                    break;
                }

                switch (Platform(platformIdx)) {
                    case MACOS:
                        item->macOSPath = path;
                        break;
                    case WINDOWS:
                        item->win32Path = path;
                        break;
                    case LINUX:
                        item->linuxPath = path;
                        break;
                    case PLATFORM_MAX: break;
                }

                auto defaultVersionEntry{item->versions.empty() ? string{"1.0.0"} : item->versions.rbegin()->first};
                ItemVersionData *versionData{nullptr};

                while (not false) {
                    std::cout << "Version to add \"" << path << "\" to [" << defaultVersionEntry << "]: " << std::flush;
                    std::getline(std::cin, input);
                    resetToPrevLine();

                    Utils::Version version{input.empty() ? defaultVersionEntry : input};
                    if (not version) {
                        std::cout << "Invalid version, try again. " << std::flush;
                        std::getline(std::cin, input);
                        resetToPrevLine();
                        continue;
                    }

                    auto versionIt{item->versions.find(version)};
                    if (versionIt == item->versions.end()) {
                        while (not false) {
                            std::cout << "Version does not exist, create it? [Y/n]: " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();

                            if (input.empty() or input == "y" or input == "Y") {
                                versionData = &item->versions.emplace(version, ItemVersionData{}).first->second;
                                newVersion = true;
                            } else if (input == "n" or input == "N") break;
                            else continue;

                            addNotesToItemVersion(itemName, *item, *versionData);
                            break;
                        }

                        if (not versionData) continue;
                    } else {
                        versionData = &versionIt->second;
                        bool exists{};
                        switch (Platform(platformIdx)) {
                            case MACOS: 
                                exists = not versionData->macOSHash.empty();
                                break;
                            case WINDOWS:
                                exists = not versionData->win32Hash.empty();
                                break;
                            case LINUX:
                                exists = not versionData->linuxHash.empty();
                                break;
                            case PLATFORM_MAX: break;
                        }
                        if (exists) {
                            std::cout << "This version is already registered for " << PLATFORM_DIRS[platformIdx] << ", choose/create another version. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }
                        versionData = &versionIt->second;
                    }

                    switch (Platform(platformIdx)) {
                        case MACOS:
                            versionData->macOSHash = hash;
                            break;
                        case WINDOWS:
                            versionData->win32Hash = hash;
                            break;
                        case LINUX:
                            versionData->linuxHash = hash;
                            break;
                        case PLATFORM_MAX:
                            break;
                    }

                    break;
                }
                std::cout << "Added " << platformStr << ' ' << typeStr << ' ' << path
                    << (newVersion ? " to new version " : " to version ")
                    << static_cast<string>(item->versions.rbegin()->first)
                    << (newItem ? string{" in new"} + (item->hidden ? " hidden" : "") + " item \"" : " in \"")
                    << itemName << '"' << '\n' << std::flush;
            }
        }
    }
}

void UpGen::addNotesToItemVersion(const string& itemName, const Item& item, ItemVersionData& versionData) {
    string input;
    if (item.hidden) {
        std::cout << "Item is hidden, skipping notes." << std::flush;
        std::getline(std::cin, input);
        resetToPrevLine();
    } else {
        auto num{0};
        while (not false) {
            std::cout << "Enter bugfix " << num << " for " << itemName << " (empty to stop): " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            if (input.empty()) break;
            versionData.fixes.emplace_back(input);
            ++num;
        }
        num = 0;
        while (not false) {
            std::cout << "Enter change " << num << " for " << itemName << " (empty to stop): " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            if (input.empty()) break;
            versionData.changes.emplace_back(input);
            ++num;
        }
        num = 0;
        while (not false) {
            std::cout << "Enter feature " << num << " for " << itemName << " (empty to stop): " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            if (input.empty()) break;
            versionData.features.emplace_back(input);
            ++num;
        }
    }
}

void UpGen::generateNewManifest(const vector<Message>& messages, const Data& data) {
    PConf::Data manifest;

    for (const auto& message : messages) {
        auto msgSect{PConf::Section::create("MESSAGE")};
        switch (message.versionComp) {
            case Update::Comparator::EQUAL:
                msgSect->label = static_cast<string>(message.version);
                break;
            case Update::Comparator::LESS_THAN:
                msgSect->label = '<' + static_cast<string>(message.version);
                break;
            case Update::Comparator::GREATER_THAN:
                msgSect->label = '>' + static_cast<string>(message.version);
                break;
        }

        if (message.fatal) msgSect->entries.emplace_back(PConf::Entry::create("FATAL"));
        msgSect->entries.emplace_back(PConf::Entry::create("CONTENT", message.message));

        manifest.emplace_back(std::move(msgSect));
    }

    for (const auto& [ id, item ] : data.items) {
        auto itemSect{PConf::Section::create(itemTypeToStr(id.type), id.name)};

        if (item.hidden) itemSect->entries.emplace_back(PConf::Entry::create("HIDDEN"));
        if (item.deprecated) itemSect->entries.emplace_back(PConf::Entry::create("DEPRECATED"));
        if (item.linuxPath) itemSect->entries.emplace_back(PConf::Entry::create(PATH_KEY_LINUX, item.linuxPath->string()));
        if (item.macOSPath) itemSect->entries.emplace_back(PConf::Entry::create(PATH_KEY_MACOS, item.macOSPath->string()));
        if (item.win32Path) itemSect->entries.emplace_back(PConf::Entry::create(PATH_KEY_WIN32, item.win32Path->string()));

        for (const auto& [ version, verData ] : item.versions) {
            auto versionSect{PConf::Section::create("VERSION", static_cast<string>(version))};

            if (item.linuxPath) versionSect->entries.emplace_back(PConf::Entry::create(HASH_KEY_LINUX, verData.linuxHash));
            if (item.macOSPath) versionSect->entries.emplace_back(PConf::Entry::create(HASH_KEY_MACOS, verData.macOSHash));
            if (item.win32Path) versionSect->entries.emplace_back(PConf::Entry::create(HASH_KEY_WIN32, verData.win32Hash));

            for (const auto& fix : verData.fixes) versionSect->entries.emplace_back(PConf::Entry::create("FIX", fix));
            for (const auto& change : verData.changes) versionSect->entries.emplace_back(PConf::Entry::create("CHANGE", change));
            for (const auto& feature : verData.features) versionSect->entries.emplace_back(PConf::Entry::create("FEAT", feature));

            itemSect->entries.emplace_back(std::move(versionSect));
        }

        manifest.emplace_back(std::move(itemSect));
    }

    for (auto bundleIt{data.bundles.rbegin()}; bundleIt != data.bundles.rend(); ++bundleIt) {
        const auto& [ version, bundle ]{*bundleIt};
        auto bundleSect{PConf::Section::create("BUNDLE", static_cast<string>(version))};

        if (not bundle.note.empty()) bundleSect->entries.emplace_back(PConf::Entry::create("NOTE", bundle.note));

        for (const auto& [ reqID, reqVer, hash] : bundle.reqs) {
            bundleSect->entries.emplace_back(PConf::Entry::create(itemTypeToStr(reqID.type), static_cast<string>(reqVer), reqID.name));
        }

        manifest.emplace_back(std::move(bundleSect));
    }

    auto outStream{Paths::openOutputFile(filepath{STAGING_DIR} / MANIFEST_DEFAULT)};
    PConf::write(outStream, manifest, nullptr);
}

void UpGen::organizeAssets(const FileMaps& fileMaps, const Data& data) {
    const auto lookupItem{[&](Platform platform, Update::ItemType type, const filepath& path, const string& hash)
        -> std::pair<string, Utils::Version> {
        for (const auto& [ itemID, item ] : data.items) {
            if (type != itemID.type) continue;

            optional<filepath> checkPath;
            switch (platform) {
                case MACOS: 
                    checkPath = item.macOSPath;
                    break;
                case WINDOWS:
                    checkPath = item.win32Path;
                    break;
                case LINUX:
                    checkPath = item.linuxPath;
                case PLATFORM_MAX:
                    abort();
            }

            if (checkPath != path) continue;

            for (const auto& [ version, versionData ] : item.versions) {
                switch (platform) {
                    case MACOS:
                        if (versionData.macOSHash != hash) continue;
                        break;
                    case WINDOWS:
                        if (versionData.win32Hash != hash) continue;
                        break;
                    case LINUX:
                        if (versionData.linuxHash != hash) continue;
                        break;
                    case PLATFORM_MAX:
                    abort();
                }

                return { itemID.name, version };
            }

            std::cout << "\nITEM LOOKUP ERR1\n" << std::flush;
            exit(1);
        }

        std::cout << "\nITEM LOOKUP ERR2\n" << std::flush;
        exit(1);
    }};

    for (auto typeIdx{0}; typeIdx < Update::TYPE_MAX; ++typeIdx) {
        auto typeFolder{Update::typeFolder(static_cast<Update::ItemType>(typeIdx))};
        auto destDir{filepath{STAGING_DIR} / "organized" / typeFolder};
        for (auto platformIdx{0}; platformIdx < PLATFORM_MAX; ++platformIdx) {
            for (const auto& [ path, hash ] : fileMaps[platformIdx][typeIdx]) {

                fs::create_directories(destDir);
                std::error_code err;
                Paths::copyOverwrite(filepath{STAGING_DIR} / PLATFORM_DIRS[platformIdx] / typeFolder / path, destDir / hash, err);
            }
        }
    }
}

cstring UpGen::itemTypeToStr(Update::ItemType type) {
    switch (type) {
        case Update::ItemType::EXEC: return "EXEC";
        case Update::ItemType::LIB: return "LIB";
        case Update::ItemType::COMP: return "COMP";
        case Update::ItemType::RSRC: return "RSRC";
        case Update::TYPE_MAX: break;
    }
    return nullptr;
}

void UpGen::listMessages(const vector<Message>& messages, bool hold) {
    clearScreen();
    std::cout << HEADER << "\nMessages:\n";

    uint32 idx{0};
    for (const auto& message : messages) {
        std::cout << "  " << idx << ") ";
        std::cout << (message.fatal ? "[F] " : "--- ");
        if (message.versionComp == Update::Comparator::LESS_THAN) std::cout << '<';
        else if (message.versionComp == Update::Comparator::GREATER_THAN) std::cout << '>';
        std::cout << static_cast<string>(message.version) << '\n';
        std::istringstream msgStream{message.message};
        string buffer;
        while (msgStream.good() and not msgStream.eof()) {
            std::getline(msgStream, buffer);
            std::cout << "        \"" << buffer << "\"\n";
        }
        std::cout << '\n';
        ++idx;
    }
    if (messages.empty()) {
        std::cout << "[NO MESSAGES]\n\n";
    }

    if (hold) {
        std::cout << "Press enter to return... " << std::flush;
        string input;
        std::getline(std::cin, input);
    }
}

void UpGen::addMessage(vector<Message>& messages) {
    clearScreen();
    std::cout << HEADER << "\n";

    string input;

    Utils::Version version;
    Update::Comparator comp{};
    string message;
    bool fatal{};

    while (not false) {
        std::cout << "Target launcher version(s): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }

        comp = Update::Comparator::EQUAL;
        if (input[0] == '<') comp = Update::Comparator::LESS_THAN;
        else if (input[0] == '>') comp = Update::Comparator::GREATER_THAN;

        if (comp == Update::Comparator::EQUAL) version = Utils::Version{input};
        else version = Utils::Version{input.substr(1)};

        if (not version) {
            resetToPrevLine() << "Invalid version entered, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        break;
    }

    while (not false) {
        std::cout << "Is this message fatal? [y/N]: " << std::flush;
        std::getline(std::cin, input);
        
        if (input.empty() or input == "n" or input == "N") fatal = false;
        else if (input == "y" or input == "Y") fatal = true;
        else {
            resetToPrevLine();
            continue;
        }
        break;
    }

    while (not false) {
        message.clear();
        std::cout << "Enter message (Finish with a line \"###\"):\n";

        uint32 linesTyped{0};
        while (not false) {
            std::cout << "> " << std::flush;
            std::getline(std::cin, input);
            ++linesTyped;
            if (input == "###") {
                if (message.empty()) {
                    std::cout << "Message must not be empty, try again. " << std::flush;
                    std::getline(std::cin, input);
                    for (auto idx{0}; idx <= linesTyped; ++idx) resetToPrevLine();
                    continue;
                } 
                break;
            }
            if (not message.empty()) message += '\n';
            message += input;
        }

        bool accepted{};
        while (not false) {
            std::cout << "Accept message? [Y/n]: " << std::flush;
            std::getline(std::cin, input);

            if (input.empty() or input == "y" or input == "Y") accepted = true;
            else if (input == "n" or input == "N") accepted = false;
            else {
                resetToPrevLine();
                continue;
            }

            break;
        }
        if (accepted) break;

        resetToPrevLine();
        for (auto idx{0}; idx < linesTyped; ++idx) resetToPrevLine();
        resetToPrevLine();
    }

    while (not false) {
        std::cout << "Confirm message add? [Y/n]: " << std::flush;
        std::getline(std::cin, input);

        if (input.empty() or input == "y" or input == "Y") {
            resetToPrevLine();
            break;
        }
        if (input == "n" or input == "N") {
            resetToPrevLine() << "Message not added. [ENTER] " << std::flush;
            std::getline(std::cin, input);
            return;
        }

        resetToPrevLine();
    }

    Message msg;
    msg.fatal = fatal;
    msg.versionComp = comp;
    msg.version = version;
    msg.message = message;
    messages.emplace_back(std::move(msg));

    std::cout << "Message added. [ENTER] " << std::flush;
    std::getline(std::cin, input);
}

void UpGen::removeMessage(vector<Message>& messages) {
    listMessages(messages, messages.empty());
    if (messages.empty()) return;

    string input;
    while (not false) {
        std::cout << "Message to remove ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }
        
        if (input == "-") return;

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid message number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto msgIdx{std::stoi(input)};
        if (msgIdx > messages.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        messages.erase(std::next(messages.begin(), msgIdx));
        std::cout << "Message deleted. [ENTER] " << std::flush;
        std::getline(std::cin, input);
        break;
    }
}

void UpGen::listItems(const Items& items, bool hold) {
    clearScreen();
    std::cout << HEADER << "\nItems:\n";

    uint32 idx{0};
    for (const auto& [ id, item ] : items) {
        std::cout << "  " << idx << ") ";
        std::cout << (item.hidden ? "[H] " : "--- ") << (item.deprecated ? "[D] " : "--- ") << id.name << " [" << itemTypeToStr(id.type) << ']' <<
            " (Latest: " << static_cast<string>(item.versions.rbegin()->first) << ")\n";
        if (item.macOSPath) std::cout << "        macOS: " << item.macOSPath.value() << '\n';
        if (item.win32Path) std::cout << "        Win32: " << item.win32Path.value() << '\n';
        if (item.linuxPath) std::cout << "        Linux: " << item.linuxPath.value() << '\n';
        std::cout << '\n';
        ++idx;
    }
    if (items.empty()) std::cout << "[NO ITEMS]" << "\n\n";

    if (hold) {
        string input;
        std::cout << "Press enter to continue... " << std::flush;
        std::getline(std::cin, input);
    }
}

void UpGen::toggleItemVisibility(Items& items) {
    listItems(items, items.empty());
    if (items.empty()) return;

    string input;
    while (not false) {
        std::cout << "Item to toggle ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }
        
        if (input == "-") return;

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid item number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto itemIdx{std::stoi(input)};
        if (itemIdx > items.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto& item{std::next(items.begin(), itemIdx)->second};
        item.hidden = not item.hidden;

        std::cout << "Item toggled. " << (item.hidden ? "(Now Hidden)" : "(Now Shown)") << " [ENTER] " << std::flush;
        std::getline(std::cin, input);
        break;
    }
}

void UpGen::toggleItemDeprecation(Items& items) {
    listItems(items, items.empty());
    if (items.empty()) return;

    string input;
    while (not false) {
        std::cout << "Item to toggle ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }
        
        if (input == "-") return;

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid item number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto itemIdx{std::stoi(input)};
        if (itemIdx > items.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto& item{std::next(items.begin(), itemIdx)->second};
        item.deprecated = not item.deprecated;

        std::cout << "Item toggled. " << (item.deprecated ? "(Now Deprecated)" : "(Now Active)") << " [ENTER] " << std::flush;
        std::getline(std::cin, input);
        break;
    }
}

void UpGen::listBundles(const Update::Bundles& bundles, bool hold) {
    clearScreen();
    std::cout << HEADER << "\nBundles:\n";

    uint32 idx{0};
    for (const auto& [ version, bundle ] : bundles) {
        std::cout << "  " << idx << ") ";
        std::cout << "--- " << static_cast<string>(version) << "\n";

        for (const auto& [ reqID, reqVer, hash] : bundle.reqs) {
            std::cout << "        [" << itemTypeToStr(reqID.type) << "] \"" << reqID.name << "\": " << static_cast<string>(reqVer) << '\n';
        }
        std::cout << '\n';
        ++idx;
    }
    if (bundles.empty()) std::cout << "[NO BUNDLES]" << "\n\n";

    if (hold) {
        string input;
        std::cout << "Press enter to continue... " << std::flush;
        std::getline(std::cin, input);
    }
}

void UpGen::addBundle(Data& data, bool fromCurrent) {
    clearScreen();
    std::cout << HEADER << "\n\n";

    Utils::Version version;
    Update::Bundle bundle;
    string input;

    while (not false) {
        std::cout << "Enter bundle version ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input == "-") return;

        version = Utils::Version{input};
        if (not version) {
            resetToPrevLine() << "Version invalid, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        if (data.bundles.contains(version)) {
            resetToPrevLine() << "Bundle exists, choose another version. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        break;
    }

    if (fromCurrent) {
        uint32 num{0};
        for (const auto& [ itemID, item ] : data.items) {
            if (item.deprecated) continue;
            bundle.reqs.emplace_back(itemID, item.versions.rbegin()->first);
            ++num;
        }
        std::cout << "Added " << num << " current items to bundle.\n";
    } else {
        auto addOfType{[&](Update::ItemType type) {
            while (not false) {
                Item *item{nullptr};
                while (not false) {
                    std::cout << "Name of " << itemTypeToStr(type) << " to add (Empty to finish): " << std::flush;
                    std::getline(std::cin, input);

                    if (input.empty()) {
                        resetToPrevLine();
                        break;
                    }
                    auto itemIt{data.items.find({ type, input })};
                    if (itemIt == data.items.end()) {
                        resetToPrevLine() << "Item doesn't exist, try again. " << std::flush;
                        std::getline(std::cin, input);
                        resetToPrevLine();
                        continue;
                    }

                    item = &itemIt->second;
                    resetToPrevLine();
                    break;
                }
                if (input.empty()) break;

                string itemName{input};
                auto latestVersion{item->versions.rbegin()->first};

                Utils::Version version;
                while (not false) {
                    auto prompt{itemName + " version [" + static_cast<string>(latestVersion) + "]: "};
                    std::cout << prompt << std::flush;
                    std::getline(std::cin, input);

                    if (input.empty()) {
                        version = latestVersion;
                    } else {
                        version = Utils::Version{input};
                        if (not version) {
                            resetToPrevLine() << "Invalid version entered, try again. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }

                        if (not item->versions.contains(version)) {
                            resetToPrevLine() << "Item doesn't have version, try again. " << std::flush;
                            std::getline(std::cin, input);
                            resetToPrevLine();
                            continue;
                        }
                    }

                    resetToPrevLine();
                    break;
                }

                bundle.reqs.emplace_back(Update::ItemID{ .type=type, .name=itemName }, version);
                std::cout << "Added " << itemTypeToStr(type) << ' ' << itemName
                    << " version " << static_cast<string>(version) << '\n' << std::flush;
            }
        }};

        addOfType(Update::ItemType::EXEC);
        addOfType(Update::ItemType::LIB);
        addOfType(Update::ItemType::COMP);
        addOfType(Update::ItemType::RSRC);
    }

    while (not false) {
        bundle.note.clear();
        std::cout << "Enter note (Finish with a line \"###\"):\n";

        uint32 linesTyped{0};
        while (not false) {
            std::cout << "> " << std::flush;
            std::getline(std::cin, input);
            ++linesTyped;
            if (input == "###") break;
            if (not bundle.note.empty()) bundle.note += '\n';
            bundle.note += input;
        }

        bool accepted{};
        while (not false) {
            std::cout << "Accept note? [Y/n]: " << std::flush;
            std::getline(std::cin, input);

            if (input.empty() or input == "y" or input == "Y") accepted = true;
            else if (input == "n" or input == "N") accepted = false;
            else {
                resetToPrevLine();
                continue;
            }

            break;
        }
        if (accepted) break;

        resetToPrevLine();
        for (auto idx{0}; idx < linesTyped; ++idx) resetToPrevLine();
        resetToPrevLine();
    }

    data.bundles.emplace(version, std::move(bundle));
    std::cout << "Added bundle " << static_cast<string>(version) << " [ENTER] " << std::flush;
    std::getline(std::cin, input);
}

void UpGen::removeBundle(Update::Bundles& bundles) {
    listBundles(bundles, bundles.empty());
    if (bundles.empty()) return;

    string input;
    while (not false) {
        std::cout << "Bundle to remove ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }

        if (input == "-") return;

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid bundle number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto bundleIdx{std::stoi(input)};
        if (bundleIdx > bundles.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        bundles.erase(std::next(bundles.begin(), bundleIdx));
        std::cout << "Bundle deleted. [ENTER] " << std::flush;
        std::getline(std::cin, input);
        break;
    }
}

void UpGen::genBundleChangelog(Data& data) {
    if (data.bundles.empty()) return;

    Utils::Version bundleVersion;
    decltype(data.bundles.begin()) bundleEndIt;
    decltype(data.bundles.begin()) bundleBeginIt;

    string input;
    while (not false) {
        listBundles(data.bundles, data.bundles.empty());
        std::cout << "Bundle to generate for ('-' to cancel): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }

        if (input == "-") return;

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid bundle number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto bundleIdx{std::stoi(input)};
        if (bundleIdx > data.bundles.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        bundleEndIt = std::next(data.bundles.begin(), bundleIdx);
        bundleVersion = bundleEndIt->first;
        ++bundleEndIt;
        break;
    }

    while (not false) {
        listBundles(data.bundles, data.bundles.empty());
        std::cout << "Bundle to generate from ('-' to list from previous): " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) {
            resetToPrevLine();
            continue;
        }

        if (not std::isdigit(input[0])) {
            resetToPrevLine() << "Invalid bundle number, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        auto bundleIdx{std::stoi(input)};
        if (bundleIdx > data.bundles.size()) {
            resetToPrevLine() << "Entry out of range, try again. " << std::flush;
            std::getline(std::cin, input);
            resetToPrevLine();
            continue;
        }

        bundleBeginIt = std::next(data.bundles.begin(), bundleIdx);
        break;
    }

    clearScreen();
    std::cout << HEADER << "\n\n";

    string notes;
    std::map<Update::ItemID, std::set<Utils::Version>> itemVersions;
    std::map<Update::ItemID, std::tuple<string, string, string>> itemInfos;

    for (auto bundleIt{bundleBeginIt}; bundleIt != bundleEndIt; ++bundleIt) {
        if (not bundleIt->second.note.empty()) {
            notes += '*' + bundleIt->second.note + "*\n\n";
        }

        for (const auto& [ reqID, reqVer, hash] : bundleIt->second.reqs) {
            auto item{data.items.find(reqID)->second};
            if (item.hidden) continue;

            if (itemVersions[reqID].contains(reqVer)) {
                continue;
            }

            auto itemVersion{item.versions.find(reqVer)->second};
            auto& [ features, changes, fixes ]{itemInfos[reqID]};
            for (const auto& feat : itemVersion.features) features += " - " + feat + '\n';
            for (const auto& change: itemVersion.changes) changes += " - " + change + '\n';
            for (const auto& fix: itemVersion.fixes) fixes += " - " + fix + '\n';

            itemVersions[reqID].emplace(reqVer);
        }
    }

    std::cout << "Version " << static_cast<string>(bundleVersion) << " of ProffieConfig includes the following updates:\n\n";
    std::cout << notes;

    for (const auto& [ id, info ] : itemInfos) {
        std::cout << "### " << id.name << "\n\n";
        const auto& [ features, changes, fixes ]{info};
        if (not features.empty()) {
            std::cout << "**New Features:**\n";
            std::cout << features << '\n';
        }
        if (not changes.empty()) {
            std::cout << "**Changes:**\n";
            std::cout << changes << '\n';
        }
        if (not fixes.empty()) {
            std::cout << "**Bugfixes:**\n";
            std::cout << fixes << '\n';
        }

        std::cout << '\n';
    }

    std::cout << "\n\nPress ENTER to continue... " << std::flush;
    std::getline(std::cin, input);
}


