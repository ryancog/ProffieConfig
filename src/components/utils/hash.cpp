#include "hash.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/hash.cpp
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

#include <iostream>

#include <tomcrypt.h>

using namespace utils::hash;

SHA256::SHA256(std::array<uint8, 32> arr) : mValue{arr} {}

SHA256 SHA256::stream(std::istream& stream) {
    std::array<uint8, 32768> buffer;

    hash_state hashState;
    sha256_init(&hashState);

    while (not false) {
        stream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        auto bytesRead{stream.gcount()};
        if (bytesRead == 0) break;

        sha256_process(&hashState, buffer.data(), bytesRead);
    }

    std::array<uint8, 32> ret;
    sha256_done(&hashState, ret.data());

    return ret;
}

std::optional<SHA256> SHA256::parseString(const std::string& str) {
    // 32-bytes (256 bits / 8 bits per byte) * 2 chars per byte
    if (str.length() != 64) return std::nullopt;

    for (char chr : str) {
        if (not std::isxdigit(chr)) return std::nullopt;
    }

    std::array<uint8, 32> ret;
    std::array<char, 3> tmpStr{0, 0, 0};
    for (auto idx{0}; idx < ret.size(); ++idx) {
        tmpStr[0] = str[(idx * 2) + 0];
        tmpStr[1] = str[(idx * 2) + 1];
        ret[idx] = strtoul(tmpStr.data(), nullptr, 16);
    }

    return ret;
}

std::array<uint8, 32> SHA256::value() const { return mValue; }

SHA256::operator std::string() const {
    std::string ret;
    ret.resize(64);

    for (size idx{0}; idx < mValue.size(); ++idx) {
        (void)snprintf(&ret[idx * 2], 3, "%02x", mValue[idx]);
    }

    return ret;
}

