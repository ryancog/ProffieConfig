#include "logger.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * logger.cpp
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

#include <iostream>
#include <unordered_map>
#include <ostream>

static std::unordered_map<std::ostream*, Logger::LogLevel>* logOutputs;

void Logger::init() {
    if (!logOutputs) logOutputs = new std::unordered_map<std::ostream*, Logger::LogLevel>{ {&std::cout, Logger::LogLevel::ALL} };
}

void Logger::log(LogLevel level, const std::string& message, const bool) {
    std::string prefix;
    switch (level) {
    case ERROR:
        prefix += "\e[31m (ERR) ";
        break;
    case WARN:
        prefix += "\e[31m(WARN) ";
        break;
    case INFO:
        prefix += "\e[32m(INFO) ";
        break;
    case DEBUG:
        prefix += "\e[36m (DBG) ";
        break;
    case VERBOSE:
        prefix += "\e[37m(VERB) ";
        break;
    default:
        std::cerr << "Invalid Log Level" << std::endl;
        return;
    }

    for (auto logOut : *logOutputs) {
        if (!(level & logOut.second)) continue;
        (*logOut.first) << prefix << message << "\e[0m" << std::endl;
    }
}

void Logger::addLogOut(std::ostream& out, LogLevel level) {
    logOutputs->insert({ &out, level});
}

void Logger::removeLogOut(std::ostream& out) {
    auto toRemove{logOutputs->find(&out)};
    if (toRemove != logOutputs->end()) logOutputs->erase(toRemove);
}


void Logger::error(const std::string& message, const bool notify) {
    log(ERROR, message, notify);
}
void Logger::warn (const std::string& message, const bool notify) {
    log(WARN, message, notify);
}
void Logger::info (const std::string& message, const bool notify) {
    log(INFO, message, notify);
}
void Logger::debug(const std::string& message, const bool notify) {
    log(DEBUG, message, notify);
}
void Logger::verbose(const std::string& message, const bool notify) {
    log(VERBOSE, message, notify);
}
