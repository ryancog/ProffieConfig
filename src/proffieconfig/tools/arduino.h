#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/tools/arduino.h
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

#include <wx/combobox.h>

#include "config/config.h"
#include "log/branch.h"

#include "../core/utilities/progress.h"

namespace Arduino {
    string version();

    struct Result {
        static constexpr cstring USAGE_MESSAGE{
            wxTRANSLATE("The configuration uses %d%% of board space. (%d/%d)")
        };

        int32 used{-1};
        int32 total{-1};
        inline float64 percent() const { return (static_cast<float64>(used) / total) * 100.0; }
    };

    /**
     * @param progress May be null
     *
     * @return result info or err string
     */
    variant<Result, string> applyToBoard(
        const string& boardPath,
        const Config::Config&,
        Progress *progress
    );

    /**
     * @return result info or err string
     */
    variant<Result, string> verifyConfig(
        const Config::Config&,
        Progress *progress
    );

    // void init(wxWindow *);
    vector<string> getBoards(Log::Branch * = nullptr);
} // namespace Arduino
