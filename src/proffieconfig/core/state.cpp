#include "state.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/core/state.cpp
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

#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>

#include "log/context.hpp"
#include "log/logger.hpp"
#include "pconf/read.hpp"
#include "pconf/write.hpp"
#include "pconf/utils.hpp"
#include "ui/dialogs/message.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/version.hpp"
#include "versions/versions.hpp"

#include "../onboard/onboard.hpp"
#include "../mainmenu/mainmenu.hpp"

namespace {

constexpr cstring LAST_VERSION_STR{"LAST_VERSION"};
constexpr cstring UPDATE_MANIFEST_STR{"UPDATE_MANIFEST"};
constexpr cstring FIRSTRUN_COMPLETE_STR{"FIRSTRUN_COMPLETE"};

utils::Version lastVersion{};

void doNecessaryMigrations();

constexpr auto NUM_BOOL_PREFS{static_cast<size>(state::prefs::Bool::Max)};
constexpr auto NUM_STR_PREFS{static_cast<size>(state::prefs::Str::Max)};
constexpr auto NUM_ENUM_PREFS{static_cast<size>(state::prefs::Enum::Max)};

std::bitset<NUM_BOOL_PREFS> boolPrefs;

// Helpers to provide names and have no default dtor to avoid errors when
// updating/adding preferences.
struct BoolPrefStrings {
    BoolPrefStrings() = delete;
    constexpr BoolPrefStrings(cstring key) : key_{key} {}

    cstring key_;
};

struct StringPrefStrings {
    StringPrefStrings() = delete;
    constexpr StringPrefStrings(cstring key, cstring def) :
        key_{key}, def_{def} {}

    cstring key_;
    cstring def_;
};

struct EnumPrefStrings {
    template <auto DEFAULT, size NUM>
    requires
        (static_cast<size>(decltype(DEFAULT)::Max) == NUM) and
        (static_cast<size>(DEFAULT) < NUM)
    static constexpr EnumPrefStrings make(
        cstring key, std::array<cstring, NUM> values
    ) {
        // TODO: Make all this less error-prone and ugly.
        // Make sure the data is stored statically. The array passed in
        // probably is treated as a temporary.
        static std::array statValues{values};
        return {key, statValues.data(), NUM, static_cast<size>(DEFAULT)};
    }

    cstring key_;
    std::span<cstring> values_;
    size def_;

private:
    constexpr EnumPrefStrings(cstring key, cstring *values, size num, size def) :
        key_{key}, values_{values, num}, def_{def} {}
};

constexpr std::array<BoolPrefStrings, NUM_BOOL_PREFS> BOOL_PREF_STRS{
    "HIDE_EDITOR_MANAGE_VERSIONS_WARN",
};

std::array<std::string, NUM_STR_PREFS> strPrefs;

constexpr std::array<StringPrefStrings, NUM_STR_PREFS> STR_PREF_STRS{{
    {
        "STYLE_EDITOR_LINK",
        "https://fredrik.hubbe.net/lightsaber/style_editor.html?S={}"
    },
}};

std::array<size, NUM_ENUM_PREFS> enumPrefs;
const std::array<EnumPrefStrings, NUM_ENUM_PREFS> ENUM_PREF_STRS{{
    EnumPrefStrings::make<state::prefs::enums::AddPresetInsertion::After_Selected>(
        "ADD_PRESET_INSERTION", std::array{
            "BEGIN",
            "END",
            "BEFORE_SEL",
            "AFTER_SEL"
        }
    )
}};

} // namespace

bool state::doneWithFirstRun{false};
// TODO: This just... isn't locked... it's fine for now, but be careful.
std::string state::manifestChannel;

void state::init() {
    loadState();

    if (not doneWithFirstRun) {
        onboard::Frame::instance = new onboard::Frame();
    } else {
        doNecessaryMigrations();
        MainMenu::instance = new MainMenu;
    }
}

bool state::prefs::get(Bool pref) {
    assert(pref < Bool::Max);
    return boolPrefs[static_cast<size>(pref)];
}

void state::prefs::set(Bool pref, bool set) {
    assert(pref < Bool::Max);
    boolPrefs[static_cast<size>(pref)] = set;
}

std::string state::prefs::get(Str pref) {
    assert(pref < Str::Max);
    return strPrefs[static_cast<size>(pref)];
}

void state::prefs::set(Str pref, std::string s) {
    assert(pref < Str::Max);
    strPrefs[static_cast<size>(pref)] = std::move(s);
}

size state::prefs::priv::get(Enum pref) {
    assert(pref < Enum::Max);
    return enumPrefs[static_cast<size>(pref)];
}

void state::prefs::priv::set(Enum pref, size e) {
    assert(pref < Enum::Max);
    enumPrefs[static_cast<size>(pref)] = e;
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

    for (size idx{0}; idx < NUM_BOOL_PREFS; ++idx) {
        if (boolPrefs[idx])
            data.push_back(pconf::Entry::create(BOOL_PREF_STRS[idx].key_));
    }

    for (size idx{0}; idx < NUM_STR_PREFS; ++idx) {
        cstring key{STR_PREF_STRS[idx].key_};
        data.push_back(pconf::Entry::create(key, strPrefs[idx]));
    }

    for (size idx{0}; idx < NUM_ENUM_PREFS; ++idx) {
        cstring key{ENUM_PREF_STRS[idx].key_};
        cstring val{ENUM_PREF_STRS[idx].values_[enumPrefs[idx]]};
        data.push_back(pconf::Entry::create(key, val));
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

    for (size idx{0}; idx < NUM_BOOL_PREFS; ++idx) {
        auto ptr{hashedData.find(BOOL_PREF_STRS[idx].key_)};
        boolPrefs[idx] = static_cast<bool>(ptr);
    }

    for (size idx{0}; idx < NUM_STR_PREFS; ++idx) {
        const auto& strings{STR_PREF_STRS[idx]};
        auto ptr{hashedData.find(strings.key_)};

        if (not ptr or not ptr->value_) {
            strPrefs[idx] = strings.def_;
            continue;
        }

        strPrefs[idx] = *ptr->value_;
    }

    for (size idx{0}; idx < NUM_ENUM_PREFS; ++idx) {
        const auto& strings{ENUM_PREF_STRS[idx]};
        auto ptr{hashedData.find(strings.key_)};

        if (not ptr or not ptr->value_) {
            enumPrefs[idx] = strings.def_;
            continue;
        }

        size valIdx{0};
        for (; valIdx < strings.values_.size(); ++valIdx) {
            if (*ptr->value_ == strings.values_[valIdx])
                break;
        }

        if (valIdx == strings.values_.size())
            // Couldn't find valid.
            enumPrefs[idx] = strings.def_;
        else
            enumPrefs[idx] = valIdx;
    }

    logger.info("Done");
}

namespace {

void doNecessaryMigrations() {
    std::error_code err;
    
    // REVIEW

    if (lastVersion.compare(utils::Version{1, 8}) < 0) {
        // Purge old ProffieOS and props data 
        fs::remove_all(paths::dataDir() / "ProffieOS", err);
        fs::remove_all(paths::dataDir() / "props", err);
        fs::remove_all(paths::resourceDir() / "props", err);
    }

    if (lastVersion.compare(utils::Version{1, 9}) < 0) {
        // Install new ProffieOS and props data 
        if (
                versions::fetch() or
                versions::installDefault(false) or
                // Get new stuffage for 7.15 installation
                versions::downloadOS({7, 15})
           ) {
            pcui::showMessage(
                _("You should visit the versions manager to retry fetching the defaults soon."),
                {
                    .caption_=_("Versions Download Failed"),
                    .style_=wxOK | wxCENTER | wxICON_WARNING
                }
            );
        }
    }

    state::saveState();
}

} // namespace

