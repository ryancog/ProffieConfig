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

struct CompileOutput {
    int32 used_;
    int32 total_;

    std::string dfuFile_;

    [[nodiscard]] float64 percent() const;
    [[nodiscard]] wxString usageMessage() const;
};

struct CompileInfo : utils::Data {
    CompileInfo(const config::Config& config) : source_{config} {}

    const config::Config& source_;

    // If this is nullopt, will recompile.
    std::optional<CompileOutput> out_;
};

void applyToBoard(
    const std::string& name,
    const std::string& boardPath,
    CompileInfo&,
    pcui::ProgressDialog& progress
);

void verifyConfig(
    const std::string& name,
    CompileInfo&,
    pcui::ProgressDialog& progress
);

std::vector<std::string> getBoards(logging::Branch * = nullptr);

#if defined(_WIN32) or defined(__linux__)
bool runDriverInstallation();
#endif

} // namespace arduino

