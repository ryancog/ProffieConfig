#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/pconf/pconf.h
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

// TODO: Break this up into separate files

#include <memory>
#include <optional>
#include <unordered_map>

#include <log/branch.h>
#include <utils/types.h>

#include "private/export.h"

namespace PConf {

struct Entry;
struct Section;

using Data = vector<std::shared_ptr<Entry>>;
using HashedData = std::unordered_multimap<string, std::shared_ptr<Entry>>;

PCONF_EXPORT bool read(std::wistream&, Data& out, Log::Branch *);
PCONF_EXPORT void write(std::wostream&, const Data&, Log::Branch *); 

[[nodiscard]] PCONF_EXPORT HashedData hash(const Data&);

enum class Type {
    ENTRY,
    SECTION
};

struct PCONF_EXPORT Entry {
    Entry() = default;
    Entry(
            string name, 
            std::optional<string> value = std::nullopt, 
            std::optional<string> label = std::nullopt, 
            std::optional<int32> labelNum = std::nullopt
         );
    virtual ~Entry() = default;

    string name;
    std::optional<string> value{std::nullopt};
    std::optional<string> label{std::nullopt};
    std::optional<int32> labelNum{std::nullopt};

    [[nodiscard]] virtual Type getType() const { return Type::ENTRY; }
};

struct PCONF_EXPORT Section : public Entry {
    Section() = default;
    Section(
            string name, 
            std::optional<string> label = std::nullopt, 
            std::optional<int32> labelNum = std::nullopt,
            Data entries = {}
           );
    Data entries;

    [[nodiscard]] Type getType() const override { return Type::SECTION; }
};

} // namespace PConf
