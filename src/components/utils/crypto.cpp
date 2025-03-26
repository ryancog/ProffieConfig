#include "crypto.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include <fstream>

#include <tomcrypt.h>

string Crypto::computeHash(const filepath& path) {
    auto inputStream{std::ifstream(path, std::ios::binary)};
    array<uint8, 32768> buffer;

    hash_state hashState;
    sha256_init(&hashState);

    while (not false) {
        inputStream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        auto bytesRead{inputStream.gcount()};
        if (bytesRead == 0) break;

        sha256_process(&hashState, buffer.data(), bytesRead);
    }
    inputStream.close();

    string hashStr;
    hashStr.resize(64);
    array<uint8_t, 32> hash;
    sha256_done(&hashState, hash.data());

    for (uint64 byte{0}; byte < hash.size(); byte++) {
        (void)snprintf(&hashStr.at(byte * 2), 3, "%02x", hash[byte]);
    }

    return hashStr;
}

