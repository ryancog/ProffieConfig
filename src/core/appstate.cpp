// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "core/appstate.h"
#include "core/defines.h"
#include "core/utilities/fileparse.h"
#include "onboard/onboard.h"
#include "mainmenu/mainmenu.h"

#include <fstream>
#include <iostream>

AppState* AppState::instance;
AppState::AppState() {}
void AppState::init() {
  instance = new AppState();
  instance->loadStateFromFile();

  if (instance->firstRun) Onboard::instance = new Onboard();
  else MainMenu::instance = new MainMenu();
}

void AppState::saveState() {
  std::ofstream stateFile(STATEFILE_PATH ".tmp");
  if (!stateFile.is_open()) {
    std::cerr << "Error creating temporary save file." << std::endl;
    stateFile.close();
    return;
  }

  stateFile << "FIRSTRUN: " << (firstRun ? "TRUE" : "FALSE") << std::endl;
  stateFile << std::endl;
  stateFile << "PROPS {" << std::endl;
  for (const auto& prop : propFileNames) {
    stateFile << "\tPROP(\"" << prop << "\")" << std::endl;
  }
  stateFile << "}" << std::endl;
  stateFile << std::endl;
  stateFile << "CONFIGS {" << std::endl;
  for (const auto& config : configFileNames) {
    stateFile << "\tCONFIG(\"" << config << "\")" << std::endl;
  }
  stateFile << "}" << std::endl;

  stateFile.close();

  remove(STATEFILE_PATH); // we don't care if it fails bc there's nothing there
  if (rename(STATEFILE_PATH ".tmp", STATEFILE_PATH) != 0) {
    std::cerr << "Error saving state file." << std::endl;
    return;
  }
}

void AppState::loadStateFromFile() {
  std::ifstream stateFile(STATEFILE_PATH);
  if (!stateFile.is_open()) {
    std::cerr << "Could not open state file, attempting recovery from tmp..." << std::endl;
    stateFile.open(STATEFILE_PATH ".tmp");
    if (!stateFile.is_open()) {
      std::cerr << "Could not open temp state file, continuing without..." << std::endl;
      propFileNames = {
        "BC",
        "caiwyn",
        "fett263",
        "sa22c",
        "shtok"
      };
      return;
    }
  }

  std::vector<std::string> state;
  std::string tmp;
  while (!stateFile.eof()) {
    getline(stateFile, tmp);
    state.push_back(tmp);
  }
  stateFile.close();

  firstRun = FileParse::parseBoolEntry("FIRSTRUN", state);
  auto tempProps = FileParse::extractSection("PROPS", state);
  for (std::string& prop : tempProps) {
    if (!(tmp = FileParse::parseLabel(prop)).empty()) propFileNames.push_back(tmp);
  }
  auto tempConfigs = FileParse::extractSection("CONFIGS", state);
  for (std::string& config : tempConfigs) {
    if (!(tmp = FileParse::parseLabel(config)).empty()) configFileNames.push_back(tmp);
  }
}

bool AppState::isSaved() {
  return saved;
}
void AppState::setSaved(bool state) {
  saved = state;
}
const std::vector<std::string>& AppState::getPropFileNames() {
  return propFileNames;
}
const std::vector<std::string>& AppState::getConfigFileNames() {
  return configFileNames;
}

void AppState::removeConfig(const std::string& configName) {
  for (auto config = configFileNames.begin(); config < configFileNames.end();) {
    if (*config == configName) config = configFileNames.erase(config);
    else config++;
  }
}
void AppState::addConfig(const std::string& configName) {
  for (const auto& config : configFileNames) if (config == configName) return;
  configFileNames.push_back(configName);
}
