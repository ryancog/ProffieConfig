#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/log/context.hpp
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

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "log/severity.hpp"

#include "log_export.h"

namespace logging {

class Logger;
class LOG_EXPORT Context {
public:
    Context() = delete;
    Context(
        std::string name,
        std::vector<std::ostream *> = {},
        bool outputToFile = true
    );

    ~Context();

    static Context& getGlobal();
    static void destroyGlobal();

    [[nodiscard]] Logger& createLogger(std::string name);
    [[nodiscard]] std::vector<Logger *> getLoggers() const;

    void setSeverity(Severity);
    void quickLog(Severity, std::string tag, std::string message);

    /**
     * Attempts to set the outputs for the global loggger.
     *
     * Must be called before first usage of `getGlobal()`.
     * Initializes the global output and cannot be used twice.
     *
     * @return true if succeeded, false otherwise
     */
    static bool setGlobalOuput(
        std::vector<std::ostream *> outStreams, bool fileOutput = true
    );

protected:
    friend class Logger;
    friend class Branch;

    void sendOut(Severity, const std::string&);
    const std::string pName;

private:
    std::vector<Logger *> mLoggers;
    std::vector<std::ostream *> mOutputs;

    std::mutex mSendLock;
    std::mutex mListLock;

    Severity mCurrentSev{Severity::Dbug};

    // Reserved object, should not be used except to maintain lifetime!
    std::ofstream mRESOutFile;
};

} // namespace logging

