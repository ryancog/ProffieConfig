#include "appstate.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include <wx/msgdlg.h>

#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "utils/paths.h"
#include "../onboard/onboard.h"

namespace AppState {

vector<string> propFileNames{};

bool doneWithFirstRun{false};
bool saved{true};

inline filepath stateFile() { return Paths::data() / ".state.pconf"; }

} // namespace AppState


void AppState::init() {
  loadState();

  if (not doneWithFirstRun) OnboardFrame::instance = new OnboardFrame();
  else MainMenu::instance = new MainMenu();
}

void AppState::saveState() {
    auto& logger{Log::Context::getGlobal().createLogger("AppState::saveState()")};

    std::ofstream stateStream(stateFile() += ".tmp");
    if (!stateStream.is_open()) {
        logger.error("Failed creating temporary save file.");
        stateStream.close();
        return;
    }

    PConf::Data data;

    if (doneWithFirstRun) data.push_back(std::make_shared<PConf::Entry>("FIRSTRUN_COMPLETE"));

    auto propSection{std::make_shared<PConf::Section>("PROPS")};
    for (const auto& prop : propFileNames) {
        propSection->entries.push_back(std::make_shared<PConf::Entry>("PROP", nullopt, prop));
    }
    data.push_back(propSection);

    PConf::write(stateStream, data, logger.bdebug("Writing save file..."));
    stateStream.close();

    std::error_code err;
    fs::remove(stateFile(), err); // we don't care if it fails bc there's nothing there
    err.clear();
    fs::rename(stateFile() += ".tmp", stateFile(), err);
    if (err.value() != 0) {
        logger.error("Failed saving state file.");
        return;
    }
}

void AppState::loadState() {
    auto& logger{Log::Context::getGlobal().createLogger("AppState::loadState()")};
    std::ifstream stateStream(stateFile());
    if (!stateStream.is_open()) {
        logger.warn("Could not open state file, attempting recovery from tmp...");
        stateStream.open(stateFile() += ".tmp");
        if (!stateStream.is_open()) {
            logger.warn("Could not open temp state file, continuing without...");
            return;
        }
    }

    PConf::Data data;
    PConf::read(stateStream, data, nullptr);
    stateStream.close();

    auto hashedData{PConf::hash(data)};
    doneWithFirstRun = hashedData.find("FIRSTRUN_COMPLETE") != hashedData.end();

    auto props = hashedData.find("PROPS");
    if (props != hashedData.end() and props->second->getType() == PConf::Type::SECTION) {
        for (const auto& prop : std::static_pointer_cast<PConf::Section>(props->second)->entries) {
            if (prop->name != "PROP") continue;
            if (not prop->label) continue;

            propFileNames.push_back(*prop->label);
        }
    }
}

bool AppState::isSaved() { return saved; }

void AppState::setSaved(bool state) {
  saved = state;
}

void AppState::addProp(const string& propName, const string& propPath, const string& propConfigPath) {
    std::ifstream configStream{propConfigPath};
    PConf::Data data;
    PConf::read(configStream, data, nullptr);
    auto hashedData{PConf::hash(data)};

    auto filenameEntry{hashedData.find("FILENAME")};
    if (filenameEntry == hashedData.end() or not filenameEntry->second->value) {
        wxMessageBox("Prop config file is invalid.", "Error Adding Prop");
        return;
    }

    auto propFileName{propPath.substr(propPath.rfind('/') + 1)};
    if (*filenameEntry->second->value != propFileName) {
        wxMessageBox("Prop config filename does not match provided prop filename", "Error Adding Prop");
        return;
    }

    fs::copy_file(propConfigPath, Paths::props() / propConfigPath.substr(propConfigPath.rfind('/') + 1), fs::copy_options::overwrite_existing);
    fs::copy_file(propPath, Paths::proffieos() / "props" / propFileName, fs::copy_options::overwrite_existing);

    propFileNames.push_back(propName);
}

void AppState::removeProp(const string& propName) {
    auto propConfigPath{Paths::props() / (propName + ".pconf")};
    if (not fs::exists(propConfigPath)) return;

    std::ifstream configStream{propName};
    PConf::Data data;
    PConf::read(configStream, data, nullptr);
    auto hashedData{PConf::hash(data)};

    auto filenameEntry{hashedData.find("FILENAME")};
    if (filenameEntry != hashedData.end() and filenameEntry->second->value) {
        fs::remove(Paths::proffieos() / "props" / *filenameEntry->second->value);
    }

    fs::remove(propConfigPath);

    for (auto propIt{propFileNames.begin()}; propIt != propFileNames.end(); ++propIt) {
        if (*propIt == propName) {
            propFileNames.erase(propIt);
            break;
        }
    }
}

const vector<string>& AppState::getPropFileNames() {
    return propFileNames;
}
