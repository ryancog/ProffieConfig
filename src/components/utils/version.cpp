#include "version.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/utils/version.cpp
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

#include <algorithm>

Utils::Version::Version(string_view str) {
    if (str.empty()) {
        err = Err::STR_EMPTY;
        return;
    }

    string_view convStr{str};
    const cstring end{str.end()};

    auto parseNum{[this, &convStr, &end]() -> uint8 {
        char *parseEnd{};
        int32 ret{};
        ret = strtol(convStr.data(), &parseEnd, 10);

        if (convStr == parseEnd) {
            err = Err::STR_INVALID;
            return 0;
        }
        convStr = parseEnd;

        if (ret >= std::numeric_limits<uint8>::max() or ret < 0) {
            err = Err::NUM_RANGE;
            return 0;
        }

        return static_cast<uint8>(ret);
    }};


    major = parseNum();
    if (err != Err::NONE or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        goto parse_label;
    } else {
        err = Err::STR_INVALID;
        return;
    }

    minor = parseNum();
    if (err != Err::NONE or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        goto parse_label;
    } else {
        err = Err::STR_INVALID;
        return;
    }

    bugfix = parseNum();
    if (err != Err::NONE or convStr.data() == end) return;
    if (convStr[0] != '-') {
        err = Err::STR_INVALID;
        return;
    }

parse_label:
    // Jump over '-'
    convStr.remove_prefix(1);
    auto labelCheckInvalid{[](char chr){
        return not std::isalpha(chr);
    }};
    if (convStr.end() != std::find_if(convStr.begin(), convStr.end(), labelCheckInvalid)) {
        err = Err::STR_INVALID;
        return;
    }

    tag = convStr;
}

Utils::Version::operator string() const {
    switch (err) {
        case Err::INVALID:
            return "INVALID";
        case Err::NUM_RANGE:
            return "Invalid range";
        case Err::STR_INVALID:
            return "Invalid input";
        case Err::STR_EMPTY:
            return "Empty input";
        case Err::NONE: break;
    }

    auto ret{std::to_string(major)};
    if (minor != NULL_REV) ret += '.' + std::to_string(minor);
    if (bugfix != NULL_REV) ret += '.' + std::to_string(bugfix);
    if (not tag.empty()) ret += '-' + tag;
    return ret;
}

Utils::Version::operator bool() const { return err == Err::NONE; }

const Utils::Version& Utils::Version::invalidObject() { 
    static const auto invalidObject{[]() {
        Version ret;
        ret.err = INVALID;
        return ret;
    }()};
    return invalidObject; 
}

