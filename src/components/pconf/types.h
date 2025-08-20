#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/pconf/types.h
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

#include "utils/types.h"

#include "pconf_export.h"

namespace PConf {

enum class Type {
    ENTRY,
    SECTION
};

struct Entry;
struct Section;

using Data = vector<std::shared_ptr<Entry>>;
struct HashedData : std::unordered_multimap<string, std::shared_ptr<Entry>> {
    [[nodiscard]] std::shared_ptr<PConf::Entry> find(const string& key) const;
    [[nodiscard]] vector<std::shared_ptr<PConf::Entry>> findAll(const string& key) const;
};

struct PCONF_EXPORT Entry {
    Entry() = default;
    Entry(
            string name, 
            optional<string> value = nullopt, 
            optional<string> label = nullopt, 
            optional<int32> labelNum = nullopt
         );
    virtual ~Entry() = default;

    string name;
    optional<string> value{nullopt};
    optional<string> label{nullopt};
    optional<int32> labelNum{nullopt};

    [[nodiscard]] virtual Type getType() const { return Type::ENTRY; }
};

struct PCONF_EXPORT Section : public Entry {
    Section() = default;
    Section(
            string name, 
            optional<string> label = nullopt, 
            optional<int32> labelNum = nullopt,
            Data entries = {}
           );
    Data entries;

    [[nodiscard]] Type getType() const override { return Type::SECTION; }
};

} // namespace PConf
