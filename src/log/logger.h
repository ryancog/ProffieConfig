#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * logger.h
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
#include <stdint.h>

namespace Logger {

enum LogLevel : uint32_t {
    ERROR,
    WARN,
    INFO,
    DEBUG,
    VERBOSE,

    ALL = ERROR | WARN | INFO | DEBUG | VERBOSE
};

void init();

void addLogOut(std::ostream&, LogLevel);
void removeLogOut(std::ostream&);

void log(LogLevel level, const std::string& message, const bool notify);

void error  (const std::string& message, const bool notify = true);
void warn   (const std::string& message, const bool notify = true);
void info   (const std::string& message, const bool notify = false);
void debug  (const std::string& message, const bool notify = false);
void verbose(const std::string& message, const bool notify = false);

}
