#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/log/branch.h
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

#include <wx/string.h>

#include "private/export.h"
#include "message.h"

namespace Log {

class Logger;

// Used to create a new logger specifically for a
// subfunction/action which is a "branch" off an
// existing logger.
//
// Extends from Message because it's placed in the
// parent logger's message list.
class LOG_EXPORT Branch : public Message {
public:
    Branch() = delete;
    Branch(const Branch&) = delete;
    ~Branch() override;

    // Create a logger with the provided branch, if valid, otherwise log
    // with the global context.
    [[nodiscard]] static Logger& optCreateLogger(wxString name, Branch *);
    [[nodiscard]] Logger& createLogger(wxString name);

    [[nodiscard]] std::list<Logger *> getLoggers() const;

    [[nodiscard]] Type getType() const override { return Type::BRANCH; }
    
private:
    friend class Logger;
    Branch(const wxString& message, Severity, Logger *);

    std::list<Logger *> mLoggers;
    std::mutex mListLock;
    const Logger *mParent;
};

} // namespace Log

