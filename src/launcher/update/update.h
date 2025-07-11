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

#include <utils/types.h>
#include <ui/message.h>
#include <log/branch.h>
#include <utils/version.h>

namespace Update {

enum ItemType {
    EXEC,
    LIB,
    COMP,
    RSRC,
    TYPE_MAX
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

    std::map<Utils::Version, ItemVersionData> versions;
    bool hidden;
};

struct Bundle {
    string note;

    struct RequiredItem {
        RequiredItem(ItemID id, Utils::Version version, string hash = {}) : id{std::move(id)}, version{std::move(version)}, hash{std::move(hash)} {}

        ItemID id;
        Utils::Version version;
        string hash;
    };
    vector<RequiredItem> reqs;
};

using Items = std::map<ItemID, Item>;
using Bundles = std::map<Utils::Version, Bundle>;

struct Data {
    Items items;
    Bundles bundles; 
};

void init();
[[nodiscard]] wxEvtHandler *getEventHandler();

filepath typeFolder(ItemType);

} // namespace Update

