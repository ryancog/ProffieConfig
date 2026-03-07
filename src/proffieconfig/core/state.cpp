#include "state.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/core/state.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>

#include <wx/msgdlg.h>

#include "log/context.hpp"
#include "log/logger.hpp"
#include "pconf/read.hpp"
#include "pconf/write.hpp"
#include "pconf/utils.hpp"
#include "ui/misc/message.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/version.hpp"

#include "../onboard/onboard.hpp"
#include "../mainmenu/mainmenu.hpp"
#include "versions/versions.hpp"

namespace {

constexpr cstring LAST_VERSION_STR{"LAST_VERSION"};
constexpr cstring UPDATE_MANIFEST_STR{"UPDATE_MANIFEST"};
constexpr cstring FIRSTRUN_COMPLETE_STR{"FIRSTRUN_COMPLETE"};

utils::Version lastVersion{};

void doNecessaryMigrations();

std::bitset<state::ePreference_Max> preferences;
constexpr std::array<cstring, state::ePreference_Max> PREFERENCE_STRS{
    "HIDE_EDITOR_MANAGE_VERSIONS_WARN"
};

} // namespace

bool state::doneWithFirstRun{false};
std::string state::manifestChannel;

void state::init() {
    loadState();

    if (not doneWithFirstRun) onboard::Frame::instance = new onboard::Frame();
    else {
        doNecessaryMigrations();
        // MainMenu::instance = new MainMenu();
    }
}

bool state::getPreference(Preference preference) {
    return preferences[preference];
}

void state::setPreference(Preference preference, bool set) {
    preferences[preference] = set;
}

void state::saveState() {
    auto& logger{logging::Context::getGlobal().createLogger("state::saveState()")};

    auto stateStream{files::openOutput(paths::stateFile() += ".tmp")};
    if (stateStream.fail()) {
        logger.error("Failed creating temporary save file.");
        stateStream.close();
        return;
    }

    pconf::Data data;
    
    data.push_back(pconf::Entry::create(
        LAST_VERSION_STR, wxSTRINGIZE(BIN_VERSION)
    ));

    if (not manifestChannel.empty()) {
        data.push_back(pconf::Entry::create(
            UPDATE_MANIFEST_STR, manifestChannel
        ));
    }

    if (doneWithFirstRun) {
        data.push_back(pconf::Entry::create(FIRSTRUN_COMPLETE_STR));
    }

    for (size idx{0}; idx < ePreference_Max; ++idx) {
        if (preferences[idx]) {
            data.push_back(pconf::Entry::create(PREFERENCE_STRS[idx]));
        }
    }

    pconf::write(stateStream, data, logger.bdebug("Writing save file..."));
    stateStream.close();

    std::error_code err;
    fs::remove(paths::stateFile(), err); // we don't care if it fails bc there's nothing there
    err.clear();
    fs::rename(paths::stateFile() += ".tmp", paths::stateFile(), err);
    if (err.value() != 0) {
        logger.error("Failed saving state file.");
        return;
    }
}

void state::loadState() {
    auto& logger{logging::Context::getGlobal().createLogger("state::loadState()")};
    auto stateStream{files::openInput(paths::stateFile())};

    if (not stateStream.is_open()) {
        logger.warn("Could not open state file, attempting recovery from tmp...");
        stateStream.open(paths::stateFile() += ".tmp");
        if (!stateStream.is_open()) {
            logger.warn("Could not open temp state file, continuing without...");
            return;
        }
    }

    logger.info("Loading state...");
    pconf::Data data;
    pconf::read(stateStream, data, nullptr);
    stateStream.close();

    auto hashedData{pconf::hash(data)};
    doneWithFirstRun = static_cast<bool>(hashedData.find(FIRSTRUN_COMPLETE_STR));
    logger.info(std::string{"Done with first run: "} + (doneWithFirstRun ? "true" : "false"));

    auto lastVersionEntry{hashedData.find(LAST_VERSION_STR)};
    if (lastVersionEntry and lastVersionEntry->value_) {
        lastVersion = utils::Version{*lastVersionEntry->value_};
    }

    auto manifestEntry{hashedData.find(UPDATE_MANIFEST_STR)};
    if (manifestEntry and manifestEntry->value_) {
        manifestChannel = *manifestEntry->value_;
    }

    for (auto idx{0}; idx < ePreference_Max; ++idx) {
        preferences[idx] = static_cast<bool>(hashedData.find(PREFERENCE_STRS[idx]));
    }

    logger.info("Done");
}

namespace {

void doNecessaryMigrations() {
    if (lastVersion.compare(utils::Version{1, 8}) < 0) {
        std::error_code err;

        // Purge old ProffieOS and props data 
        fs::remove_all(paths::dataDir() / "ProffieOS", err);
        fs::remove_all(paths::dataDir() / "props", err);
        fs::remove_all(paths::resourceDir() / "props", err);

        // Install new ProffieOS and props data 
        if (versions::fetch() or versions::installDefault(true)) {
            pcui::showMessage(
                _("Versions Download Failed"),
                _("You should visit the versions manager to retry fetching the defaults soon."),
                wxOK | wxCENTER | wxICON_WARNING
            );
        }
    }

    state::saveState();
}

} // namespace

