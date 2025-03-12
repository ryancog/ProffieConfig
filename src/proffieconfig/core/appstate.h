#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "utils/types.h"

#include <wx/window.h>

class AppState {
public:
  static void init();
  static AppState* instance;

  bool isSaved();
  void setSaved(bool = true);
  void loadState();
  void saveState();

  void addProp(const string& propName, const string& propPath, const string& propConfigPath);
  void removeProp(const string& propName);

  const vector<string>& getPropFileNames();
  static constexpr array<string_view, 5> BUILTIN_PROPS{
      "BC",
      "caiwyn",
      "fett263",
      "sa22c",
      "shtok",
  };

  bool doneWithFirstRun{false};

private:
  AppState() = default;
  AppState(const AppState&) = delete;

  vector<string> propFileNames{};

  bool saved{true};
};
