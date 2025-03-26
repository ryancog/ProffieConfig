#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/log/context.h
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
#include <vector>
#include <fstream>

#include <wx/string.h>

#include "utils/types.h"

#include "severity.h"
#include "private/export.h"

namespace Log {

class Logger;
class LOG_EXPORT Context {
public:
    Context() = delete;
    Context(wxString name, vector<std::wostream *> = {}, bool outputToFile = true);
    ~Context();

    static Context& getGlobal();
    static void destroyGlobal();

    [[nodiscard]] Logger& createLogger(wxString name);
    [[nodiscard]] list<Logger *> getLoggers() const;

    void setSeverity(Severity);
    void quickLog(Severity, wxString tag, wxString message);

    /**
     * Attempts to set the outputs for the global loggger.
     *
     * Must be called before first usage of `getGlobal()`.
     * Initializes the global output and cannot be used twice.
     *
     * @return true if succeeded, false otherwise
     */
    static bool setGlobalOuput(vector<std::wostream *> outStreams, bool fileOutput = true);

protected:
    friend class Logger;
    friend class Branch;

    void sendOut(Severity, const wxString&);
    const wxString pName;

private:
    std::list<Logger *> mLoggers;
    std::vector<std::wostream *> mOutputs;

    std::mutex mSendLock;
    std::mutex mListLock;

    Severity mCurrentSev{Severity::DBUG};

    // Reserved object, should not be used except to maintain lifetime!
    std::wofstream mRESOutFile;
};

} // namespace Log

