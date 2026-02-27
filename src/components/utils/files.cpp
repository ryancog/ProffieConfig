#include "files.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/utils/files.cpp
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

#ifdef _WIN32
#include <errhandlingapi.h>
// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __attribute__((dllimport)) int CopyFileA(const char *, const char *, int);
#endif

bool files::copyOverwrite(const fs::path& src, const fs::path& dst, std::error_code& err) {
#   ifdef _WIN32
    auto res{CopyFileA(src.string().c_str(), dst.string().c_str(), false)};
    err = {static_cast<int>(GetLastError()), std::system_category()};
    return res;
#   else
    return fs::copy_file(src, dst, fs::copy_options::overwrite_existing, err);
#   endif
}

std::ifstream files::openInput(const fs::path& path) {
    // POSIX Specifies that there is no distinction between binary/text modes.
    // So this shouldn't make a difference on good operating systems.
    //
    // On Windows, however, MSVC's STL implementation just kills itself because
    // it can't handle its own stupid CRLF endings and tellg/seekg plainly do
    // not work.
    // See: https://github.com/microsoft/STL/issues/1784
    return std::ifstream{path, std::ios::binary | std::ios::in};
}

std::ofstream files::openOutput(const fs::path& path) {
    return std::ofstream{path, std::ios::binary | std::ios::out};
}

