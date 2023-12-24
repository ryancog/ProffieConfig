// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/appstate.h"
#include "core/defines.h"
#include "core/utilities/fileparse.h"
#include "onboard/onboard.h"
#include "editor/editorwindow.h"
#include "config/configuration.h"

#include <fstream>
#include <iostream>

AppState* AppState::instance;
AppState::AppState() {}
void AppState::init() {
  instance = new AppState();
  instance->loadStateFromFile();

  instance->firstRun = false;
  if (instance->firstRun) {
    instance->onboard = new Onboard();
  } else {
    EditorWindow::instance = new EditorWindow();
    Configuration::readConfig();
  }
}

void AppState::saveState() {
  std::ofstream stateFile(STATEFILE_PATH ".tmp");
  if (!stateFile.is_open()) {
    std::cerr << "Error creating temporary save file." << std::endl;
    return;
  }

  stateFile << "FIRSTRUN: " << (firstRun ? "TRUE" : "FALSE") << std::endl;
  stateFile << "PROPS: {" << std::endl;
  for (const auto& prop : propFileNames) {
    stateFile << "\tPROP(\"" << prop << "\")" << std::endl;
  }
  stateFile << "}" << std::endl;
  stateFile.close();

  if (rename(STATEFILE_PATH ".tmp", STATEFILE_PATH) != 0) {
    std::cerr << "Error saving state file." << std::endl;
  }
}

void AppState::loadStateFromFile() {
  std::ifstream stateFile(STATEFILE_PATH);
  if (!stateFile.is_open()) {
    std::cerr << "Could not open state file, attempting recovery from tmp..." << std::endl;
    stateFile.open(STATEFILE_PATH ".tmp");
    if (!stateFile.is_open()) {
      std::cerr << "Could not open temp state file, continuing without..." << std::endl;
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
std::vector<PropFile*>& AppState::getProps() {
  return props;
}

void AppState::clearProps() {
  props.clear();
}

void AppState::addProp(PropFile* prop) {
  props.push_back(prop);
}
