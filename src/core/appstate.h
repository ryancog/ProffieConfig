// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include <string>
#include <vector>

class AppState {
public:
  static void init();
  static AppState* instance;

  bool isSaved();
  void setSaved(bool = true);
  void loadStateFromFile();
  void saveState();
  const std::vector<std::string>& getPropFileNames();

  bool firstRun{true};

private:
  AppState();
  AppState(const AppState&) = delete;

  std::vector<std::string> propFileNames;

  bool saved{true};

};
