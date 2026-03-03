#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/process/process.hpp
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

#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "utils/types.hpp"

#include "process_export.h"

struct PROCESS_EXPORT Process {
    ~Process();

    struct Result {
        enum {
            eSuccess,
            eCreation_Failed,
            eConnection_Failed,
            eExecution_Failed,

            eCrashed,
            // systemResult holds exit() value
            eExited_With_Failure,
            // See systemResult_
            eUnknown,
        } err_;

        int64 systemResult_{0};
    };

    /**
     * Create a new process
     */
    void create(std::string exec, std::span<std::string> args = {});

#   ifdef _WIN32
    static Result elevatedProcess(
        cstring exec, const std::span<std::string>& args = {}
    );
#   endif


    std::optional<std::string> read();
    bool write(const std::string_view&);

    Result finish();

private:
    void *mRef{nullptr};
};

