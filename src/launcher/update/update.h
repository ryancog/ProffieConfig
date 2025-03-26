#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/update.h
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

#include <map>
#include <type_traits>

#include <utils/types.h>
#include <ui/message.h>
#include <log/branch.h>

namespace Update {

enum ItemType {
    EXEC,
    LIB,
    COMP,
    RSRC,
    TYPE_MAX
};

struct Version {
    Version() = default;

    Version(const std::string& str) : Version(std::string_view{str}) {}
    Version(const std::wstring& str) : Version(std::wstring_view{str}) {}

    /**
     * Construct a Version from a string.
     *
     * Uses the form: "[major](.[minor])(.[bugfix])(-[tag])".
     * - "-[tag]", minor, and bugfix are optional
     * - tag is a string unbroken by space
     * - major, minor, and bugfix are positive ints [0-255]
     */
    template<typename CHAR_TYPE>
    Version(std::basic_string_view<CHAR_TYPE> str) {
        std::basic_string_view<CHAR_TYPE> convStr{str};
        const CHAR_TYPE *const end{str.end()};

        auto parseNum{[this, &convStr, &end]() -> uint8 {
            if (convStr.data() >= end) {
                err = Err::STR_EMPTY;
                return 0;
            }

            CHAR_TYPE *parseEnd{};
            int32 ret{};
            if constexpr (std::is_same_v<CHAR_TYPE, char>) {
                ret = strtol(convStr.data(), &parseEnd, 10);
            } else if constexpr (std::is_same_v<CHAR_TYPE, wchar_t>) {
                ret = wcstol(convStr.data(), &parseEnd, 10);
            } else {
                static_assert(false);
            }

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

    static Version invalidObject();

    uint8 major{0};
    uint8 minor{0};
    uint8 bugfix{0};

    // Version is valid if `err == NONE`
    enum Err : uint8 {
        NONE,
        INVALID,
        NUM_RANGE,
        STR_INVALID,
        STR_EMPTY,
    } err{Err::NONE};

    /**
     * Used for (optional) tagging of specialized versions. e.g 1.0.0-dev.
     * "dev" would be the tag.
     */
    string tag;

    auto operator<=>(const Version&) const = default;

    /**
     * Convert a Version into string representation.
     *
     * If valid, it returns [major](.[minor])(.[bugfix])(-[tag]).
     * If invalid, it returns a string form of the `Err`
     */
    operator string() const;
    operator bool() const;
};

enum class Comparator {
    EQUAL,
    LESS_THAN,
    GREATER_THAN,
};

struct ItemID {
    ItemType type;
    string name;
    bool ignored{false};

    auto operator<=>(const ItemID&) const = default;
};

struct ItemVersionData {
    string hash;

    vector<string> fixes;
    vector<string> changes;
    vector<string> features;
};

struct Item {
    string path;

    std::map<Version, ItemVersionData> versions;
    bool hidden;
};

struct Bundle {
    string note;

    struct RequiredItem {
        RequiredItem(ItemID id, Version version, string hash = {}) : id{std::move(id)}, version{std::move(version)}, hash{std::move(hash)} {}

        ItemID id;
        Version version;
        string hash;
    };
    vector<RequiredItem> reqs;
};

using Items = std::map<ItemID, Item>;
using Bundles = std::map<Version, Bundle>;

struct Data {
    Items items;
    Bundles bundles; 
};

void init();
[[nodiscard]] wxEvtHandler *getEventHandler();

filepath typeFolder(ItemType);

} // namespace Update

