#include "branch.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include "context.h"
#include "logger.h"

namespace Log {

} // namespace Log

Log::Branch::Branch (const std::string& message, Severity sev, Logger *logger) :
    Message(message, sev, logger), mParent(logger) {}

Log::Branch::~Branch() {
    for (const auto *logger : mLoggers) {
        delete logger;
    }
}

Log::Logger& Log::Branch::createLogger(std::string name) {
    mListLock.lock();
    mLoggers.push_back(new Logger{std::move(name), mParent->pContext});
    mListLock.unlock();
    return *mLoggers.back();
}

Log::Logger& Log::Branch::optCreateLogger(std::string name, Log::Branch *branch) {
    return branch ? branch->createLogger(std::move(name)) : Context::getGlobal().createLogger(std::move(name));
}

std::list<Log::Logger *> Log::Branch::getLoggers() const { return mLoggers; }


