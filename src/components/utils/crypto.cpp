#include "crypto.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/crypto.cpp
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

#include <random>
#include <tomcrypt.h>

std::mt19937_64& Crypto::randGen() {
    static std::mt19937_64 gen{std::random_device{}()};
    return gen;
}

Crypto::Hash::Hash(array<uint8, 32> arr) : mValue{arr} {}

Crypto::Hash Crypto::Hash::stream(std::istream& stream) {
    array<uint8, 32768> buffer;

    hash_state hashState;
    sha256_init(&hashState);

    while (not false) {
        stream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        auto bytesRead{stream.gcount()};
        if (bytesRead == 0) break;

        sha256_process(&hashState, buffer.data(), bytesRead);
    }

    array<uint8, 32> ret;
    sha256_done(&hashState, ret.data());

    return ret;
}

optional<Crypto::Hash> Crypto::Hash::parseString(const string& str) {
    // 32-bytes (256 bits / 8 bits per byte) * 2 chars per byte
    if (str.length() != 64) return nullopt;

    for (char chr : str) {
        if (not std::isxdigit(chr)) return nullopt;
    }

    array<uint8, 32> ret;
    array<char, 3> tmpStr{0, 0, 0};
    for (auto idx{0}; idx < ret.size(); ++idx) {
        tmpStr[0] = str[(idx * 2) + 0];
        tmpStr[1] = str[(idx * 2) + 1];
        ret[idx] = strtoul(tmpStr.data(), nullptr, 16);
    }

    return ret;
}

array<uint8, 32> Crypto::Hash::value() const { return mValue; }

Crypto::Hash::operator string() const {
    string ret;
    ret.resize(64);

    for (size idx{0}; idx < mValue.size(); ++idx) {
        (void)snprintf(&ret[idx * 2], 3, "%02x", mValue[idx]);
    }

    return ret;
}


