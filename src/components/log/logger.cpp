#include "logger.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/log/private/logger.cpp
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

#include "branch.h"
#include "context.h"

namespace Log {

} // namespace Log

Log::Logger::Logger(string name, Context *context) :
    name(std::move(name)), pContext(context) {}

Log::Logger::~Logger() {
    for (const auto *message : mMessages) {
        delete message;
    }
}

[[nodiscard]] vector<Log::Message *> Log::Logger::getMessages() const {
    return mMessages;
}

Log::Branch* Log::Logger::branch(Severity sev, const string& message) {
    auto *branch{new Log::Branch(message, sev, this)};
    auto formattedMessage{branch->formatted()};

    pContext->sendOut(sev, formattedMessage);

    mMessageLock.lock();
    mMessages.push_back(branch);
    mMessageLock.unlock();

    return branch;
}

Log::Branch* Log::Logger::berror(const string& message) {
    return branch(Severity::ERR, message);
}

Log::Branch* Log::Logger::bwarn(const string& message) {
    return branch(Severity::WARN, message);
}

Log::Branch* Log::Logger::binfo(const string& message) {
    return branch(Severity::INFO, message);
}

Log::Branch* Log::Logger::bdebug(const string& message) {
    return branch(Severity::DBUG, message);
}

Log::Branch* Log::Logger::bverbose(const string& message) {
    return branch(Severity::VERB, message);
}

void Log::Logger::log(Severity sev, const string& message) {
    auto *messageObj{new Message(message, sev, this)};
    auto formattedMessage{messageObj->formatted()};

    pContext->sendOut(sev, formattedMessage);

    mMessageLock.lock();
    mMessages.push_back(messageObj);
    mMessageLock.unlock();
}

void Log::Logger::error(const string& message) {
    return log(Severity::ERR, message);
}
void Log::Logger::warn(const string& message) {
    return log(Severity::WARN, message);
}
void Log::Logger::info(const string& message) {
    return log(Severity::INFO, message);
}
void Log::Logger::debug(const string& message) {
    return log(Severity::DBUG, message);
}
void Log::Logger::verbose(const string& message) {
    return log(Severity::VERB, message);
}

