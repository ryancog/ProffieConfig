#include "types.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/pconf/types.cpp
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

PConf::Entry::Entry(
    string name, 
    optional<string> value,
    optional<string> label,
    optional<int32> labelNum
) : name(std::move(name)),
    value(std::move(value)),
    label(std::move(label)),
    labelNum(labelNum) {}

PConf::Section::Section(
    string name, 
    optional<string> label, 
    optional<int32> labelNum,
    Data entries
) : Entry(
        std::move(name),
        nullopt,
        std::move(label),
        labelNum
    ),
    entries(std::move(entries)) {}

PConf::EntryPtr::EntryPtr(const SectionPtr& ptr) : shared_ptr(ptr) {}
PConf::EntryPtr::EntryPtr(SectionPtr&& ptr) : shared_ptr(std::move(ptr)) {}

PConf::EntryPtr PConf::Entry::create(
    string name,
    optional<string> value,
    optional<string> label,
    optional<int32> labelNum
) {
    return EntryPtr(new Entry(std::move(name), std::move(value), std::move(label), labelNum));
}

PConf::SectionPtr PConf::Section::create(
    string name,
    optional<string> label,
    optional<int32> labelNum,
    Data entries
) {
    return SectionPtr(new Section(std::move(name), std::move(label), labelNum, std::move(entries)));
}

void PConf::HashedData::erase(const IndexedEntryPtr& entry) {
    unordered_multimap::erase(entry.iter);
}

PConf::HashedData::IndexedEntryPtr PConf::HashedData::find(const string& key) const {
    const auto iter{unordered_multimap::find(key)};
    if (iter != end()) return { iter->second, iter };
    return { nullptr, iter };
}

vector<PConf::HashedData::IndexedEntryPtr> PConf::HashedData::findAll(const string& key) const {
    vector<PConf::HashedData::IndexedEntryPtr> ret;
    auto [iter, end]{unordered_multimap::equal_range(key)};
    for (; iter != end; ++iter) {
        ret.emplace_back(iter->second, iter);
    }
    return ret;
}

