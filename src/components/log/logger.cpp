#include "logger.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
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

#include "log/branch.hpp"
#include "log/context.hpp"

using namespace logging;

Logger::Logger(std::string name, Context *context) :
    name_(std::move(name)), pContext(context) {}

Logger::~Logger() {
    for (const auto *message : mMessages) {
        delete message;
    }
}

[[nodiscard]] std::vector<Message *> Logger::getMessages() const {
    return mMessages;
}

Branch* Logger::branch(Severity sev, const std::string& message) {
    auto *branch{new Branch(message, sev, this)};
    auto formattedMessage{branch->formatted()};

    pContext->sendOut(sev, formattedMessage);

    mMessageLock.lock();
    mMessages.push_back(branch);
    mMessageLock.unlock();

    return branch;
}

Branch* Logger::berror(const std::string& message) {
    return branch(Severity::Err, message);
}

Branch* Logger::bwarn(const std::string& message) {
    return branch(Severity::Warn, message);
}

Branch* Logger::binfo(const std::string& message) {
    return branch(Severity::Info, message);
}

Branch* Logger::bdebug(const std::string& message) {
    return branch(Severity::Dbug, message);
}

Branch* Logger::bverbose(const std::string& message) {
    return branch(Severity::Verb, message);
}

void Logger::log(Severity sev, const std::string& message) {
    auto *messageObj{new Message(message, sev, this)};
    auto formattedMessage{messageObj->formatted()};

    pContext->sendOut(sev, formattedMessage);

    mMessageLock.lock();
    mMessages.push_back(messageObj);
    mMessageLock.unlock();
}

void Logger::error(const std::string& message) {
    log(Severity::Err, message);
}

void Logger::warn(const std::string& message) {
    log(Severity::Warn, message);
}

void Logger::info(const std::string& message) {
    log(Severity::Info, message);
}

void Logger::debug(const std::string& message) {
    log(Severity::Dbug, message);
}

void Logger::verbose(const std::string& message) {
    log(Severity::Verb, message);
}

