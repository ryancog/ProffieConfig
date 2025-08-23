#include "message.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include "log/severity.h"
#include "logger.h"

namespace Log {

} // namespace Log

Log::Message::Message(string message, Severity severity, Logger *parent) :
        message(std::move(message)), severity(severity), logTag(parent->name) {}

string Log::Message::formatted() const {
    string messagePrefix;
    switch (severity) {
        case Severity::VERB:
            messagePrefix += "(VERB) ";
            break;
        case Severity::DBUG:
            messagePrefix += "(DBUG) ";
            break;
        case Severity::INFO:
            messagePrefix += "(INFO) ";
            break;
        case Severity::WARN:
            messagePrefix += "(WARN) ";
            break;
        case Severity::ERR:
            messagePrefix += " (ERR) ";
            break;
        case Severity::MAX:
            abort();
    }
    messagePrefix += {"[" + logTag + "] "};
    auto tmpMessage{message};
    auto newlinePos{tmpMessage.find('\n')};
    while (newlinePos != string::npos) {
        tmpMessage.insert(newlinePos + 1, messagePrefix.size(), ' ');
        newlinePos = tmpMessage.find('\n', newlinePos + 1);
    }

    return messagePrefix + tmpMessage;
}


