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

#include <charconv>

#include "utils/string.h"

Utils::Version::Version(string_view str) {
    if (str.empty()) {
        err = Err::STR_EMPTY;
        return;
    }

    string_view convStr{str};
    const cstring end{str.end()};

    auto parseNum{[this, &convStr, &end]<typename T>(T& val) {
        auto res{std::from_chars(
            convStr.data(),
            convStr.data() + convStr.length(),
            val
        )};

        // Yeah this is ugly to check if there was an error...
        // par for the c++stdlib course
        if (res.ec != std::errc{}) {
            if (res.ec == std::errc::result_out_of_range) {
                err = Err::NUM_RANGE;
            } else if (res.ec == std::errc::invalid_argument) {
                err = Err::STR_INVALID;
            } else {
                err = Err::UNKNOWN;
            }
            return;
        }
        convStr = res.ptr;
    }};

    const auto parseLabel{[this, &convStr]() {
        // Jump over '-'
        convStr.remove_prefix(1);

        tag = convStr;
        uint32 numTrimmed{};
        trimUnsafe(tag, &numTrimmed);

        if (numTrimmed) {
            err = Err::STR_INVALID;
            return;
        }
    }};


    parseNum(major);
    if (err != Err::NONE or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        parseLabel();
        return;
    } else {
        err = Err::STR_INVALID;
        return;
    }

    parseNum(minor);
    if (err != Err::NONE or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        parseLabel();
        return;
    } else {
        err = Err::STR_INVALID;
        return;
    }

    parseNum(bugfix);
    if (err != Err::NONE or convStr.data() == end) return;
    if (convStr[0] != '-') {
        err = Err::STR_INVALID;
        return;
    }

    parseLabel();
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
        case Err::UNKNOWN:
            return "Unknown (Parse) Error";
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

