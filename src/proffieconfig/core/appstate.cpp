#include "appstate.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <bitset>
#include <filesystem>
#include <fstream>

#include <wx/msgdlg.h>

#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/paths.h"
#include "utils/version.h"

#include "../onboard/onboard.h"

namespace {

constexpr cstring LAST_VERSION_STR{"LAST_VERSION"};
constexpr cstring UPDATE_MANIFEST_STR{"UPDATE_MANIFEST"};
constexpr cstring FIRSTRUN_COMPLETE_STR{"FIRSTRUN_COMPLETE"};

Utils::Version lastVersion{};

void doNecessaryMigrations();

std::bitset<AppState::PREFERENCE_MAX> preferences;
constexpr array<cstring, AppState::PREFERENCE_MAX> PREFERENCE_STRS{
    "HIDE_EDITOR_MANAGE_VERSIONS_WARN"
};

} // namespace

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
    
    data.push_back(PConf::Entry::create(LAST_VERSION_STR, wxSTRINGIZE(BIN_VERSION)));
    if (not manifestChannel.empty()) data.push_back(PConf::Entry::create(UPDATE_MANIFEST_STR, manifestChannel));
    if (doneWithFirstRun) data.push_back(PConf::Entry::create(FIRSTRUN_COMPLETE_STR));

    for (auto idx{0}; idx < PREFERENCE_MAX; ++idx) {
        if (preferences[idx]) {
            data.push_back(PConf::Entry::create(PREFERENCE_STRS[idx]));
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
    doneWithFirstRun = static_cast<bool>(hashedData.find(FIRSTRUN_COMPLETE_STR));
    logger.info(string{"Done with first run: "} + (doneWithFirstRun ? "true" : "false"));

    auto lastVersionIter{hashedData.find("LAST_VERSION")};
    if (lastVersionIter and lastVersionIter->value) lastVersion = Utils::Version{*lastVersionIter->value};

    for (auto idx{0}; idx < PREFERENCE_MAX; ++idx) {
        preferences[idx] = static_cast<bool>(hashedData.find(PREFERENCE_STRS[idx]));
    }

    logger.info("Done");
}

namespace {

void doNecessaryMigrations() {
    if (lastVersion < Utils::Version{1, 8}) {
        std::error_code err;
        fs::remove_all(Paths::resourceDir() / "props", err);
        fs::remove_all(Paths::osDir(), err);

        // TODO: Try to download new stuffage
    }

    AppState::saveState();
}

} // namespace

