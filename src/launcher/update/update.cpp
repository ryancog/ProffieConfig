#include "update.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/update.cpp
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

#include <tomcrypt.h>

#include <utils/types.h>

namespace Update {

wxEvtHandler *handler{nullptr};

} // namespace Update


void Update::init() {
    handler = new wxEvtHandler();
}

wxEvtHandler *Update::getEventHandler() { return handler; }

Update::Version::Version(string_view str) {
    string_view convStr{str};
    const cstring end{str.end()};

    auto parseNum{[this, &convStr, &end]() -> uint8 {
        if (convStr.data() >= end) {
            err = Err::STR_EMPTY;
            return 0;
        }

        char *parseEnd{};
        int32 ret{};
        ret = strtol(convStr.data(), &parseEnd, 10);

        if (convStr == parseEnd) {
            err = Err::STR_INVALID;
            return 0;
        }
        convStr = parseEnd;

        if (ret > std::numeric_limits<uint8>::max() or ret < 0) {
            err = Err::NUM_RANGE;
            return 0;
        }

        return static_cast<uint8>(ret);
    }};


    major = parseNum();
    if (err != Err::NONE) return;

    // Jump over '.'
    convStr.remove_prefix(1);
    minor = parseNum();
    if (err != Err::NONE) {
        minor = 0;
        if (err == Err::STR_EMPTY) err = Err::NONE;
        return;
    }

    // Jump over '.'
    convStr.remove_prefix(1);
    bugfix = parseNum();
    if (err != Err::NONE) {
        bugfix = 0;
        if (err == Err::STR_EMPTY) err = Err::NONE;
        return;
    }

    if (convStr == end) return;
    if (convStr[0] != '-') {
        err = Err::STR_INVALID;
        return;
    }

    // Jump over '-'
    convStr.remove_prefix(1);
    // If has whitespace
    if (convStr.end() != std::find_if(convStr.begin(), convStr.end(), [](char chr){ return std::isspace(chr); })) {
        err = Err::STR_INVALID;
        return;
    }

    tag = convStr;
}

Update::Version::operator string() const {
    switch (err) {
        case Err::INVALID:
            return "INVALID";
        case Err::NUM_RANGE:
            return "Invalid given numeric range";
        case Err::STR_INVALID:
            return "Invalid input";
        case Err::STR_EMPTY:
            return "Empty input";
        case Err::NONE: break;
    }

    auto ret{std::to_string(major)};
    ret += '.' + std::to_string(minor);
    ret += '.' + std::to_string(bugfix);
    if (not tag.empty()) ret += '-' + tag;
    return ret;
}

Update::Version::operator bool() const { return err == Err::NONE; }

Update::Version Update::Version::invalidObject() {
    Version ret;
    ret.err = Err::INVALID;
    return ret;
}

filepath Update::typeFolder(ItemType type) {
    switch (type) {
        case ItemType::EXEC: return "bin";
        case ItemType::LIB:  return "lib";
        case ItemType::COMP: return "components";
        case ItemType::RSRC: return "resources";
        case TYPE_MAX: break;
    }
    return {};
}

