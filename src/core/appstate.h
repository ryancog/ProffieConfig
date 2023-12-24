// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/config/propfile.h"
#include "onboard/onboard.h"
#include <string>
#include <vector>

#pragma once

class AppState {
public:
  static void init();
  static AppState* instance;

  Onboard* onboard;
  bool isSaved();
  void setSaved(bool = true);
  void loadStateFromFile();
  void saveState();
  void clearProps();
  void addProp(PropFile*);
  const std::vector<std::string>& getPropFileNames();
  std::vector<PropFile*>& getProps();

private:
  AppState();
  AppState(const AppState&) = delete;

  bool firstRun{true};
  std::vector<std::string> propFileNames;
  std::vector<PropFile*> props;

  bool saved{true};

};
