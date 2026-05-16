#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/tools/serialmonitor.hpp
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

#include <functional>
#include <mutex>
#include <semaphore>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

#include "utils/types.hpp"

struct Error {
    enum class Code {
        None,
        Disconnected,
        Unknown,
    } rsn_{Code::None};

    /**
     * Platform-specific error code.
     */
    int32 code_{0};

    // Was there an error?
    explicit operator bool() const { return rsn_ != Code::None; }
};

struct SerialMonitor {
    SerialMonitor();
    ~SerialMonitor();

    [[nodiscard]] bool isOpen() const;

    void setOnDisconnect(std::function<void()>);

    [[nodiscard]] Error open(const std::string&);
    void close();

    [[nodiscard]] Error write(std::string_view);
    [[nodiscard]] Error read(char&);

private:
    void devLoop();

    mutable std::recursive_mutex mMutex;

    std::thread mDevThread;
    std::binary_semaphore mSmphr{0};

    std::function<void()> mOnDisconnect;

#   if defined(__APPLE__) or defined(__linux__)
    int mFd{-1};
#   elif defined(_WIN32)
    HANDLE mHandle{INVALID_HANDLE_VALUE};
#   endif
};

