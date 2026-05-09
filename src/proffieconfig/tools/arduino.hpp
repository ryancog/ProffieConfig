#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/tools/arduino.hpp
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

#include <vector>
#include <string>

#include <wx/combobox.h>

#include "config/config.hpp"
#include "log/branch.hpp"
#include "ui/dialogs/progress.hpp"

namespace arduino {

std::string version();

void applyToBoard(
    const std::string& boardPath,
    const config::Config&,
    pcui::ProgressDialog& progress
);

void verifyConfig(
    const config::Config&,
    pcui::ProgressDialog& progress
);

std::vector<std::string> getBoards(logging::Branch * = nullptr);

#if defined(_WIN32) or defined(__linux__)
bool runDriverInstallation();
#endif

} // namespace arduino

