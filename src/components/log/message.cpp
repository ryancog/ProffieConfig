#include "message.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/log/private/message.cpp
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

#include "logging/severity.hpp"
#include "logging/logger.hpp"

using namespace logging;

Message::Message(std::string message, Severity severity, Logger *parent) :
        message_(std::move(message)),
        severity_(severity),
        logTag_(parent->name_) {}

std::string Message::formatted() const {
    std::string messagePrefix;

    switch (severity_) {
        using enum Severity;
        case Verb:
            messagePrefix += "(VERB) ";
            break;
        case Dbug:
            messagePrefix += "(DBUG) ";
            break;
        case Info:
            messagePrefix += "(INFO) ";
            break;
        case Warn:
            messagePrefix += "(WARN) ";
            break;
        case Err:
            messagePrefix += " (ERR) ";
            break;
        case Max:
            abort();
    }

    messagePrefix += {"[" + logTag_ + "] "};
    auto tmpMessage{message_};
    auto newlinePos{tmpMessage.find('\n')};

    while (newlinePos != std::string::npos) {
        tmpMessage.insert(newlinePos + 1, messagePrefix.size(), ' ');
        newlinePos = tmpMessage.find('\n', newlinePos + 1);
    }

    return messagePrefix + tmpMessage;
}


