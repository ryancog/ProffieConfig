#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/core/state.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#include <wx/window.h>

namespace state {

void init();

extern bool doneWithFirstRun;
extern std::string manifestChannel;

// Default is off
enum Preference {
    ePreference_Hide_Editor_Manage_Versions_Warn,
    ePreference_Max
};

enum {
    eID_Main_Menu,
    eID_Editor,
    eID_Versions_Manager,
};

bool getPreference(Preference);
void setPreference(Preference, bool);

void loadState();
void saveState();

} // namespace state

