#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/process/process.h
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

#include "utils/types.h"

#include "process_export.h"

struct PROCESS_EXPORT Process {
    ~Process();

    struct Result {
        enum {
            SUCCESS,
            CREATION_FAILED,
            CONNECTION_FAILED,
            EXECUTION_FAILED,

            CRASHED,
            // systemResult holds exit() value
            EXITED_WITH_FAILURE,
            // See systemResult
            UNKNOWN,
        } err;
        int64 systemResult{0};
    };

    /**
     * Create a new process
     */
    void create(cstring exec, const span<string>& args = {});

#   ifdef _WIN32
    static Result elevatedProcess(
        cstring exec, const span<string>& args = {}
    );
#   endif


    optional<string> read();
    bool write(const string_view&);

    Result finish();

private:
    void *mRef{nullptr};
};

