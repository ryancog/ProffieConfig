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
    EntryPtr() = default;
    EntryPtr(std::nullptr_t ptr) : shared_ptr(ptr) {}
    explicit EntryPtr(Entry *ptr) : shared_ptr(ptr) {}
    EntryPtr(const SectionPtr& ptr);
    EntryPtr(SectionPtr&& ptr);

    [[nodiscard]] SectionPtr section() const {
        return std::dynamic_pointer_cast<Section>(*this);
    }
};

using Data = vector<EntryPtr>;
struct HashedData : std::unordered_multimap<string, EntryPtr> {
    struct IndexedEntryPtr : EntryPtr {
        const const_iterator iter;
    };

    void erase(const IndexedEntryPtr&);

    [[nodiscard]] IndexedEntryPtr find(const string& key) const;
    [[nodiscard]] vector<IndexedEntryPtr> findAll(const string& key) const;
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
