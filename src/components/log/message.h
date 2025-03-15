#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/log/message.h
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

#include "private/export.h"
#include "severity.h"

namespace Log {

class Logger;
class LOG_EXPORT Message {
public:
    Message() = delete;
    virtual ~Message() = default;

    enum class Type {
        MESSAGE,
        BRANCH
    };

    [[nodiscard]] virtual Type getType() const { return Type::MESSAGE; }

    [[nodiscard]] std::string formatted() const;
    const std::string message;
    const Severity severity;
    const std::string logTag;

protected:
    friend class Logger;
    friend class Context;
    Message(std::string message, Severity severity, Logger *parent);
    Message(std::string message, Severity severity, std::string logTag) :
        message(std::move(message)), severity(severity), logTag(logTag) {}
};

} // namespace Log

