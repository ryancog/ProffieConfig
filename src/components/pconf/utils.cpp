#include "utils.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/pconf/utils.cpp
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

PConf::HashedData PConf::hash(const Data& data) {
    HashedData ret;
    for (const auto& entry : data) {
        ret.emplace(entry->name, entry);
    }

    return ret;
}

vector<string> PConf::valueAsList(const optional<string>& optStr) {
    if (not optStr) return {};

    vector<string> ret;
    auto str{*optStr};

    while (true) {
        const auto end{str.find('\n')};
        ret.push_back(str.substr(0, end));

        if (end == string::npos) break;
        str = str.substr(end + 1);
    }

    return ret;
}

optional<string> PConf::listAsValue(const vector<string>& list) {
    if (list.empty()) return nullopt;

    string ret;
    for (const auto& str : list) {
        ret += str;
        ret += '\n';
    }
    // Pop last newline
    ret.pop_back();

    return ret;
}

