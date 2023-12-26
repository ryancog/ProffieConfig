// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"

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

private:
  AppState();
  AppState(const AppState&) = delete;

  bool firstRun{true};
  std::vector<std::string> propFileNames;
  std::vector<EditorWindow*> editors;

  bool saved{true};

};
