#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/window.h>

#include "utils/types.h"

namespace AppState {

void init();

extern bool doneWithFirstRun;
extern string manifestChannel;

// Default is off
enum Preference {
    HIDE_EDITOR_MANAGE_VERSIONS_WARN,
    PREFERENCE_MAX
};

enum {
    ID_MainMenu,
    ID_VersionsManager,
};

bool getPreference(Preference);
void setPreference(Preference, bool);

void loadState();
void saveState();

} // namespace AppState
