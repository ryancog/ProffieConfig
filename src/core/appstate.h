// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/propfile.h"
#include <string>
#include <vector>

#pragma once

class AppState {
public:
  static void init();
  static AppState* instance;


  bool isSaved();
  void setSaved(bool = true);
  void loadStateFromFile();
  void saveState();
  void addProp(const PropFile&);
  const std::vector<std::string>& getProps();

private:
  AppState();
  AppState(const AppState&) = delete;

  bool firstRun{true};
  std::vector<std::string> propNames;
  std::vector<PropFile> props;

  bool saved{true};

};
