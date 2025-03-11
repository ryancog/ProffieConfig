#include "appstate.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "utils/paths.h"
#include "../onboard/onboard.h"
#include "../core/defines.h"

#include <fstream>
#include <iostream>
#include <memory>

AppState* AppState::instance;

static inline filepath stateFile() { return Paths::data() / ".state.pconf"; }

void AppState::init() {
  instance = new AppState();
  instance->loadState();

  if (not instance->doneWithFirstRun) OnboardFrame::instance = new OnboardFrame();
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

    auto props= hashedData.find("PROPS");
    if (props->second->getType() == PConf::Type::SECTION) {
        for (const auto prop : std::static_pointer_cast<PConf::Section>(props->second)->entries) {
            if (prop->name != "PROP") continue;
            if (not prop->label) continue;

            propFileNames.push_back(*prop->label);
        }
    }
}

bool AppState::isSaved() {
  return saved;
}

void AppState::setSaved(bool state) {
  saved = state;
}

void AppState::addProp(const string& propName, const string& propPath, const string& propConfigPath) {
    fs::copy_file(propConfigPath, Paths::props() / propConfigPath.substr(propConfigPath.rfind('/') + 1));
    fs::copy_file(propPath, Paths::data() / "ProffieOS" / "props" / propPath.substr(propPath.rfind('/') + 1));
    propFileNames.push_back(propName);
}

const vector<string>& AppState::getPropFileNames() {
    return propFileNames;
}
