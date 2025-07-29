#include "context.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include "app/app.h"
#include "paths/paths.h"
#include "utils/types.h"

#include "logger.h"

namespace Log {

constexpr cstring GLOBAL_TAG{"GLOBAL"};
std::unique_ptr<Log::Context> globalContext;

} // namespace Log

Log::Context::Context(string name, vector<std::ostream *> outStreams, bool outputToFile)
    : pName(std::move(name)), mOutputs(std::move(outStreams)) {

  if (outputToFile) {
    if (pName == GLOBAL_TAG) {
      mRESOutFile.open(Paths::logs() / (App::getAppName() + ".log"));
    } else {
      mRESOutFile.open(Paths::logs() / (App::getAppName() + "-" + pName + ".log"));
    }
    mOutputs.insert(mOutputs.begin(), &mRESOutFile);

    constexpr cstring HEADER_START{" Log [Context: "};
    constexpr cstring HEADER_END{"]\n"};
    constexpr cstring TIME_START{"Started at "};

    auto now{std::chrono::system_clock::now()};
    auto timeNow{std::chrono::system_clock::to_time_t(now)};

    mRESOutFile << App::getAppName().c_str() << " Log (" << wxSTRINGIZE(BIN_VERSION)
                << ") [Context: " << pName << "]\n";
    mRESOutFile << "Started at " << std::ctime(&timeNow) << "\n\n"
                << std::flush;
  }
}

Log::Context::~Context() {
  for (const auto *logger : mLoggers) {
    delete logger;
  }
  mRESOutFile.close();
}

Log::Context &Log::Context::getGlobal() {
  if (not globalContext) {
    globalContext = std::make_unique<Log::Context>(GLOBAL_TAG);
  }
  return *globalContext;
}

void Log::Context::destroyGlobal() { globalContext.reset(); }

bool Log::Context::setGlobalOuput(vector<std::ostream *> outStreams,
                                  bool fileOutput) {
  if (globalContext)
    return false;

  globalContext = std::make_unique<Log::Context>(
      GLOBAL_TAG, std::move(outStreams), fileOutput);
  return true;
}

void Log::Context::setSeverity(Severity sev) { mCurrentSev = sev; }

void Log::Context::quickLog(Severity sev, string tag, string message) {
    sendOut(sev, Message{std::move(message), sev, std::move(tag)}.formatted());
}

void Log::Context::sendOut(Severity sev, const string &str) {
    if (sev < mCurrentSev) return;

    mSendLock.lock();
    for (auto *out : mOutputs) *out << '\n' << str << std::flush;
    mSendLock.unlock();
}

Log::Logger &Log::Context::createLogger(string name) {
    mListLock.lock();
    mLoggers.push_back(new Logger{std::move(name), this});
    mListLock.unlock();
    return *mLoggers.back();
}

vector<Log::Logger *> Log::Context::getLoggers() const { return mLoggers; }
