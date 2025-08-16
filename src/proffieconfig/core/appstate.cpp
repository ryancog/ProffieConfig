#include "appstate.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <bitset>
#include <filesystem>
#include <fstream>
#include <memory>

#include <wx/msgdlg.h>

#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "pconf/read.h"
#include "pconf/utils.h"
#include "pconf/write.h"
#include "utils/paths.h"
#include "utils/version.h"

#include "../onboard/onboard.h"

namespace AppState {

Utils::Version lastVersion{};

void doNecessaryMigrations();

std::bitset<PREFERENCE_MAX> preferences;
constexpr array<cstring, PREFERENCE_MAX> PREFERENCE_STRS{
    "HIDE_EDITOR_MANAGE_VERSIONS_WARN"
};

} // namespace AppState

bool AppState::doneWithFirstRun{false};
string AppState::manifestChannel;

void AppState::init() {
    loadState();

    if (not doneWithFirstRun) OnboardFrame::instance = new OnboardFrame();
    else {
        doNecessaryMigrations();
        MainMenu::instance = new MainMenu();
    }
}

bool AppState::getPreference(Preference preference) {
    return preferences[preference];
}

void AppState::setPreference(Preference preference, bool set) {
    preferences[preference] = set;
}

void AppState::saveState() {
    auto& logger{Log::Context::getGlobal().createLogger("AppState::saveState()")};

    std::ofstream stateStream(Paths::stateFile() += ".tmp");
    if (!stateStream.is_open()) {
        logger.error("Failed creating temporary save file.");
        stateStream.close();
        return;
    }

    PConf::Data data;
    
    data.push_back(std::make_shared<PConf::Entry>("LAST_VERSION", wxSTRINGIZE(BIN_VERSION)));
    if (not manifestChannel.empty()) data.push_back(std::make_shared<PConf::Entry>("UPDATE_MANIFEST", manifestChannel));
    if (doneWithFirstRun) data.push_back(std::make_shared<PConf::Entry>("FIRSTRUN_COMPLETE"));

    for (auto idx{0}; idx < PREFERENCE_MAX; ++idx) {
        if (preferences[idx]) {
            data.push_back(std::make_shared<PConf::Entry>(PREFERENCE_STRS[idx]));
        }
    }

    PConf::write(stateStream, data, logger.bdebug("Writing save file..."));
    stateStream.close();

    std::error_code err;
    fs::remove(Paths::stateFile(), err); // we don't care if it fails bc there's nothing there
    err.clear();
    fs::rename(Paths::stateFile() += ".tmp", Paths::stateFile(), err);
    if (err.value() != 0) {
        logger.error("Failed saving state file.");
        return;
    }
}

void AppState::loadState() {
    auto& logger{Log::Context::getGlobal().createLogger("AppState::loadState()")};
    std::ifstream stateStream(Paths::stateFile());

    if (not stateStream.is_open()) {
        logger.warn("Could not open state file, attempting recovery from tmp...");
        stateStream.open(Paths::stateFile() += ".tmp");
        if (!stateStream.is_open()) {
            logger.warn("Could not open temp state file, continuing without...");
            return;
        }
    }

    logger.info("Loading state...");
    PConf::Data data;
    PConf::read(stateStream, data, nullptr);
    stateStream.close();

    auto hashedData{PConf::hash(data)};
    doneWithFirstRun = hashedData.find("FIRSTRUN_COMPLETE") != hashedData.end();
    logger.info(string{"Done with first run: "} + (doneWithFirstRun ? "true" : "false"));

    auto lastVersionIter{hashedData.find("LAST_VERSION")};
    if (lastVersionIter != hashedData.end() and lastVersionIter->second->value) lastVersion = Utils::Version{lastVersionIter->second->value.value()};

    for (auto idx{0}; idx < PREFERENCE_MAX; ++idx) {
        preferences[idx] = hashedData.find(PREFERENCE_STRS[idx]) != hashedData.end();
    }

    logger.info("Done");
}

void AppState::doNecessaryMigrations() {
    if (lastVersion < Utils::Version{"1.8.0"}) {
        std::error_code err;
        fs::remove_all(Paths::resourceDir() / "props", err);
        fs::remove_all(Paths::osDir(), err);

        // TODO: Try to download new stuffage
    }

    saveState();
}

