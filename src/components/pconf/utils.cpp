#include "utils.hpp"
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

pconf::HashedData pconf::hash(const Data& data) {
    HashedData ret;
    for (const auto& entry : data) {
        ret[entry->name_].push_back(entry);
    }

    return ret;
}

std::vector<std::string> pconf::valueAsList(
    const std::optional<std::string>& optStr
) {
    if (not optStr) return {};

    std::vector<std::string> ret;
    auto str{*optStr};

    while (true) {
        const auto end{str.find('\n')};
        ret.push_back(str.substr(0, end));

        if (end == std::string::npos) break;
        str = str.substr(end + 1);
    }

    return ret;
}

std::optional<std::string> pconf::listAsValue(
    const std::vector<std::string>& list
) {
    if (list.empty()) return std::nullopt;

    std::string ret;
    for (const auto& str : list) {
        ret += str;
        ret += '\n';
    }
    // Pop last newline
    ret.pop_back();

    return ret;
}

