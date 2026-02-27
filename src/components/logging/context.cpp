#include "context.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/log/context.cpp
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

#include <ctime>
#include <memory>
#include <utility>
#include <string>

#include <wx/app.h>

#include "logging/logger.hpp"
#include "utils/paths.hpp"
#include "utils/types.hpp"

using namespace logging;

namespace {

constexpr cstring GLOBAL_TAG{"GLOBAL"};
std::unique_ptr<Context> globalContext;

} // namespace

Context::Context(
    std::string name,
    std::vector<std::ostream *> outStreams,
    bool outputToFile
) : pName(std::move(name)), mOutputs(std::move(outStreams)) {

    if (outputToFile) {
        const auto appName{
            wxApp::GetGUIInstance()->GetAppName().ToStdString()
        };

        fs::create_directories(paths::logDir());
        if (pName == GLOBAL_TAG) {
            mRESOutFile.open(paths::logDir() / (appName + ".log"));
        } else {
            const auto filename{appName + "-" + pName + ".log"};
            mRESOutFile.open(paths::logDir() / filename);
        }
        mOutputs.insert(mOutputs.begin(), &mRESOutFile);

        constexpr cstring HEADER_START{" Log [Context: "};
        constexpr cstring HEADER_END{"]\n"};
        constexpr cstring TIME_START{"Started at "};

        auto now{std::chrono::system_clock::now()};
        auto timeNow{std::chrono::system_clock::to_time_t(now)};

        mRESOutFile << appName.c_str() << " Log (" << wxSTRINGIZE(BIN_VERSION)
            << ") [Context: " << pName << "]\n";
        mRESOutFile << "Started at " << std::ctime(&timeNow);
        mRESOutFile << "\n" << std::flush;
    }
}

Context::~Context() {
    for (const auto *logger : mLoggers) {
        delete logger;
    }
    mRESOutFile.close();
}

Context &Context::getGlobal() {
    if (not globalContext) {
        globalContext = std::make_unique<Context>(GLOBAL_TAG);
    }
    return *globalContext;
}

void Context::destroyGlobal() { globalContext.reset(); }

bool Context::setGlobalOuput(
    std::vector<std::ostream *> outStreams,
    bool fileOutput
) {
    if (globalContext) return false;

    globalContext = std::make_unique<Context>(
        GLOBAL_TAG,
        std::move(outStreams),
        fileOutput
    );
    return true;
}

void Context::setSeverity(Severity sev) { mCurrentSev = sev; }

void Context::quickLog(Severity sev, std::string tag, std::string message) {
    sendOut(
        sev,
        Message{
            std::move(message),
            sev,
            std::move(tag)
        }.formatted()
    );
}

void Context::sendOut(Severity sev, const std::string &str) {
    if (sev < mCurrentSev) return;

    mSendLock.lock();
    for (auto *out : mOutputs) *out << '\n' << str << std::flush;
    mSendLock.unlock();
}

Logger &Context::createLogger(std::string name) {
    mListLock.lock();
    mLoggers.push_back(new Logger{std::move(name), this});
    mListLock.unlock();
    return *mLoggers.back();
}

std::vector<Logger *> Context::getLoggers() const { return mLoggers; }

