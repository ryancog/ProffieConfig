#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "utils/types.h"

#include <wx/window.h>

namespace AppState {

void init();

[[nodiscard]] bool isSaved();
extern bool doneWithFirstRun;

void setSaved(bool = true);
void loadState();
void saveState();

void addProp(const string& propName, const string& propPath, const string& propConfigPath);
void removeProp(const string& propName);

const vector<string>& getPropFileNames();
static constexpr array<cstring, 5> BUILTIN_PROPS{
    "BC",
    "caiwyn",
    "fett263",
    "sa22c",
    "shtok",
};

} // namespace AppState
