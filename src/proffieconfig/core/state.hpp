#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/core/state.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "utils/types.hpp"

namespace state {

void init();

extern bool doneWithFirstRun;
extern std::string manifestChannel;

namespace prefs {

// Default is off
enum class Bool : size {
    Hide_Editor_Manage_Versions_Warn,
    Max
};

template <Bool PREF>
struct BoolData {

};

enum class Str : size {
    Style_Editor_Link,
    Max
};

enum class Enum : size {
    Add_Preset_Insertion,
    Handle_Keep_Save_Files,
    Max,
};

namespace enums {

enum class AddPresetInsertion {
    Begin,
    End,
    Before_Selected,
    After_Selected,
    Max,
};

enum class HandleKeepSaveFiles {
    Ignore_Alert,
    Ignore,
    Allow_Hide_Unused,
    Allow,
    Max,
};

template <Enum ENUM>
struct Data;

template <>
struct Data<Enum::Add_Preset_Insertion> {
    using Values = AddPresetInsertion;
};

template <>
struct Data<Enum::Handle_Keep_Save_Files> {
    using Values = HandleKeepSaveFiles;
};

} // namespace enums

bool get(Bool);
void set(Bool, bool);

std::string get(Str);
void set(Str, std::string);

namespace priv {

size get(Enum);
void set(Enum, size);

} // namespace priv

template <Enum ENUM>
auto get() {
    return static_cast<enums::Data<ENUM>::Values>(priv::get(ENUM));
}

template <Enum ENUM>
void set(typename enums::Data<ENUM>::Values v) {
    priv::set(ENUM, static_cast<size>(v));
}

} // namespace prefs

enum {
    eID_Main_Menu,
    eID_Onboard,
    eID_Editor,
    eID_Versions_Manager,
};

void loadState();
void saveState();

} // namespace state

