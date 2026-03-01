#include "branch.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/log/branch.cpp
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

#include "log/context.hpp"
#include "log/logger.hpp"

using namespace logging;

Branch::Branch(const std::string& message, Severity sev, Logger *logger) :
    Message(message, sev, logger), mParent(logger) {}

Branch::~Branch() {
    for (const auto *logger : mLoggers) {
        delete logger;
    }
}

Logger& Branch::createLogger(std::string name) {
    std::lock_guard scopeLock{mListLock};
    mLoggers.push_back(new Logger{
        std::move(name),
        mParent->pContext
    });
    return *mLoggers.back();
}

Logger& Branch::optCreateLogger(std::string name, Branch *branch) {
    return branch
        ? branch->createLogger(std::move(name))
        : Context::getGlobal().createLogger(std::move(name));
}

std::vector<Logger *> Branch::getLoggers() const { return mLoggers; }

