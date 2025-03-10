#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "core/types.h"

class AppState {
public:
  static void init();
  static AppState* instance;

  void addConfig(const string&);
  void removeConfig(const string&);
  bool isSaved();
  void setSaved(bool = true);
  void loadStateFromFile();
  void saveState();

  vector<string> propFileNames{};
  const vector<string>& getConfigFileNames();

  bool firstRun{true};

private:
  AppState();
  AppState(const AppState&) = delete;

  vector<string> configFileNames;

  bool saved{true};
};
