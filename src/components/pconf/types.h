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

using SectionPtr = std::shared_ptr<Section>;

struct PCONF_EXPORT EntryPtr : std::shared_ptr<Entry> {
    using shared_ptr::shared_ptr;

    [[nodiscard]] SectionPtr section() const {
        return std::dynamic_pointer_cast<Section>(*this);
    }
};

using Data = vector<EntryPtr>;
/*
 * On Windows, the multimap stores equal-keyed items in reverse order, and
 * according to the C++ stdlib spec the order of all the items is not
 * explicitly defined, even within equal (simply that they're contiguous)
 *
 * A map w/ vec properly conveys the intent.
 */
struct PCONF_EXPORT HashedData {
    void erase(const EntryPtr&);

    [[nodiscard]] EntryPtr find(const string& key) const;
    [[nodiscard]] vector<EntryPtr> findAll(const string& key) const;

    vector<EntryPtr>& operator[](const string& key);

private:
    std::unordered_map<string, vector<EntryPtr>> mMap;
};

struct PCONF_EXPORT Entry {
    [[nodiscard]] static EntryPtr create(
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

private:
    friend Section;
    Entry(
        string name,
        optional<string> value,
        optional<string> label,
        optional<int32> labelNum
    );
};

struct PCONF_EXPORT Section : public Entry {
    [[nodiscard]] static SectionPtr create(
        string name,
        optional<string> label = nullopt,
        optional<int32> labelNum = nullopt,
        Data entries = {}
    );

    Data entries;

private:
    Section(
        string name, 
        optional<string> label,
        optional<int32> labelNum,
        Data entries
   );
};

} // namespace PConf
