#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/log/logger.h
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

#include <list>
#include <mutex>
#include <string>

#include <wx/string.h>

#include "private/export.h"
#include "message.h"
#include "severity.h"

namespace Log {
class Branch;
class Context;

// Primary logger class
class LOG_EXPORT Logger {
public:
    Logger() = delete;
    Logger(const Logger&) = delete;
    ~Logger();

    const wxString name;

    [[nodiscard]] Branch *branch(Severity, const wxString& message);
    [[nodiscard]] Branch *berror(const wxString& message);
    [[nodiscard]] Branch *bwarn(const wxString& message);
    [[nodiscard]] Branch *binfo(const wxString& message);
    [[nodiscard]] Branch *bdebug(const wxString& message);
    [[nodiscard]] Branch *bverbose(const wxString& message);

    void log(Severity, const wxString&);
    void error(const wxString&);
    void warn(const wxString&);
    void info(const wxString&);
    void debug(const wxString&);
    void verbose(const wxString&);

    // Copy to avoid concurrent read/mod errors
    [[nodiscard]] std::list<Message *> getMessages() const;

protected:
    friend class Branch;
    friend class Context;

    Logger(wxString name, Context *context);
    Context *const pContext;

private:

    std::list<Message *> mMessages;
    std::mutex mMessageLock;
};

} // namespace Log

