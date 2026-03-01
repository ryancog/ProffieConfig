#include "types.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

pconf::Entry::Entry(
    std::string name, 
    std::optional<std::string> value,
    std::optional<std::string> label,
    std::optional<uint32> labelNum
) : name_(std::move(name)),
    value_(std::move(value)),
    label_(std::move(label)),
    labelNum_(labelNum) {}

pconf::Section::Section(
    std::string name, 
    std::optional<std::string> label, 
    std::optional<uint32> labelNum,
    Data entries
) : Entry(
        std::move(name),
        std::nullopt,
        std::move(label),
        labelNum
    ),
    entries_(std::move(entries)) {}

pconf::EntryPtr pconf::Entry::create(
    std::string name,
    std::optional<std::string> value,
    std::optional<std::string> label,
    std::optional<uint32> labelNum
) {
    return EntryPtr{new Entry(
        std::move(name),
        std::move(value),
        std::move(label),
        labelNum
    )};
}

pconf::SectionPtr pconf::Section::create(
    std::string name,
    std::optional<std::string> label,
    std::optional<uint32> labelNum,
    Data entries
) {
    return SectionPtr(new Section(
        std::move(name),
        std::move(label),
        labelNum,
        std::move(entries)
    ));
}

void pconf::HashedData::erase(const EntryPtr& entry) {
    auto vecIter{mMap.find(entry->name_)};
    if (vecIter == mMap.end()) return;

    auto iter{vecIter->second.begin()};
    const auto endIter{vecIter->second.end()};
    for (; iter != endIter; ++iter) {
        if (iter->get() == entry.get()) {
            vecIter->second.erase(iter);
            break;
        }
    }

    if (vecIter->second.empty()) {
        mMap.erase(vecIter);
    }
}

pconf::EntryPtr pconf::HashedData::find(const std::string& key) const {
    const auto vecIter{mMap.find(key)};
    if (vecIter == mMap.end()) return nullptr;

    /*
     * If the vec exists, it's guaranteed to have at least one item.
     */
    return vecIter->second[0];
}

std::vector<pconf::EntryPtr> pconf::HashedData::findAll(
    const std::string& key
) const {
    const auto vecIter{mMap.find(key)};
    if (vecIter == mMap.end()) return {};

    return vecIter->second;
}

std::vector<pconf::EntryPtr>& pconf::HashedData::operator[](
    const std::string& key
) {
    return mMap[key];
}

