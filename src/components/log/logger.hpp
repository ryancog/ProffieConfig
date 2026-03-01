#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/log/logger.hpp
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

#include <mutex>
#include <string>
#include <vector>

#include "logging/message.hpp"
#include "logging/severity.hpp"

#include "log_export.h"

namespace logging {

class Branch;
class Context;

// Primary logger class
class LOG_EXPORT Logger {
public:
    Logger() = delete;
    Logger(const Logger&) = delete;
    ~Logger();

    const std::string name_;

    [[nodiscard]] Branch *branch(Severity, const std::string& message);
    [[nodiscard]] Branch *berror(const std::string& message);
    [[nodiscard]] Branch *bwarn(const std::string& message);
    [[nodiscard]] Branch *binfo(const std::string& message);
    [[nodiscard]] Branch *bdebug(const std::string& message);
    [[nodiscard]] Branch *bverbose(const std::string& message);

    void log(Severity, const std::string&);
    void error(const std::string&);
    void warn(const std::string&);
    void info(const std::string&);
    void debug(const std::string&);
    void verbose(const std::string&);

    // Copy to avoid concurrent read/mod errors
    [[nodiscard]] std::vector<Message *> getMessages() const;

protected:
    friend class Branch;
    friend class Context;

    Logger(std::string name, Context *context);
    Context *const pContext;

private:

    std::vector<Message *> mMessages;
    std::mutex mMessageLock;
};

} // namespace logging

