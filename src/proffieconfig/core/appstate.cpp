#include "appstate.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

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
#include "ui/message.h"
#include "paths/paths.h"
#include "utils/version.h"

#include "../onboard/onboard.h"

namespace AppState {

vector<string> propFileNames{};

Utils::Version lastVersion{};

} // namespace AppState

bool AppState::doneWithFirstRun{false};
string AppState::manifestChannel;

void AppState::init() {
    loadState();

    if (lastVersion < Utils::Version{"1.8.0"}) {
        std::error_code err;
        fs::remove_all(Paths::resources() / "props", err);
        fs::remove_all(Paths::proffieos(), err);

        // TODO: Try to download new stuffage
    }

    saveState();

    if (not doneWithFirstRun) OnboardFrame::instance = new OnboardFrame();
    else MainMenu::instance = new MainMenu();
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

    auto propSection{std::make_shared<PConf::Section>("PROPS")};
    for (const auto& prop : propFileNames) {
        propSection->entries.push_back(std::make_shared<PConf::Entry>("PROP", nullopt, prop));
    }
    data.push_back(propSection);

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
    if (!stateStream.is_open()) {
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

    auto props = hashedData.find("PROPS");
    if (props != hashedData.end() and props->second->getType() == PConf::Type::SECTION) {
        for (const auto& prop : std::static_pointer_cast<PConf::Section>(props->second)->entries) {
            if (prop->name != "PROP") continue;
            if (not prop->label) continue;

            logger.info("Read prop: " + *prop->label);
            propFileNames.emplace_back(*prop->label);
        }
    }
    logger.info("Done");
}

void AppState::addProp(const string& propName, const string& propPath, const string& propConfigPath) {
    std::ifstream configStream{propConfigPath};
    PConf::Data data;
    PConf::read(configStream, data, nullptr);
    auto hashedData{PConf::hash(data)};

    auto filenameEntry{hashedData.find("FILENAME")};
    if (filenameEntry == hashedData.end() or not filenameEntry->second->value) {
        PCUI::showMessage(_("Prop config file is invalid."), _("Error Adding Prop"));
        return;
    }

    auto propFileName{propPath.substr(propPath.rfind('/') + 1)};
    if (*filenameEntry->second->value != propFileName) {
        PCUI::showMessage(_("Prop config filename does not match provided prop filename"), _("Error Adding Prop"));
        return;
    }

    fs::copy_file(propConfigPath, Paths::props() / propConfigPath.substr(propConfigPath.rfind('/') + 1), fs::copy_options::overwrite_existing);
    fs::copy_file(propPath, Paths::proffieos() / "props" / propFileName, fs::copy_options::overwrite_existing);

    propFileNames.emplace_back(propName);
}

void AppState::removeProp(const string& propName) {
    auto& logger{Log::Context::getGlobal().createLogger("AppState::removeProp()")};
    auto propConfigPath{Paths::props() / (propName + ".pconf")};

    logger.info("Removing prop \"" + propName + '"');

    std::ifstream configStream{propConfigPath};
    PConf::Data data;
    logger.debug("Reading prop pconf \"" + propConfigPath.string() + '"');
    PConf::read(configStream, data, nullptr);
    auto hashedData{PConf::hash(data)};

    auto filenameEntry{hashedData.find("FILENAME")};
    if (filenameEntry != hashedData.end() and filenameEntry->second->value) {
        logger.debug("Removing prop file \"" + *filenameEntry->second->value + '"');
        fs::remove(Paths::proffieos() / "props" / *filenameEntry->second->value);
    }

    logger.debug("Removing prop pconf");
    fs::remove(propConfigPath);

    for (auto propIt{propFileNames.begin()}; propIt != propFileNames.end(); ++propIt) {
        if (*propIt == propName) {
            propFileNames.erase(propIt);
            break;
        }
    }

    logger.debug("Done");
}

const vector<string>& AppState::getPropFileNames() {
    return propFileNames;
}
