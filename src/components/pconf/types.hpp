#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/pconf/types.hpp
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

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/types.hpp"

#include "pconf_export.h"

namespace pconf {

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

using Data = std::vector<EntryPtr>;
/*
 * On Windows, the multimap stores equal-keyed items in reverse order, and
 * according to the C++ stdlib spec the order of all the items is not
 * explicitly defined, even within equal (simply that they're contiguous)
 *
 * A map w/ vec properly conveys the intent.
 */
struct PCONF_EXPORT HashedData {
    void erase(const EntryPtr&);

    [[nodiscard]] EntryPtr find(const std::string& key) const;
    [[nodiscard]] std::vector<EntryPtr> findAll(const std::string& key) const;

    std::vector<EntryPtr>& operator[](const std::string& key);

private:
    std::unordered_map<std::string, std::vector<EntryPtr>> mMap;
};

struct PCONF_EXPORT Entry {
    [[nodiscard]] static EntryPtr create(
        std::string name,
        std::optional<std::string> value = std::nullopt,
        std::optional<std::string> label = std::nullopt,
        std::optional<uint32> labelNum = std::nullopt
    );
    virtual ~Entry() = default;

    std::string name_;
    std::optional<std::string> value_{std::nullopt};
    std::optional<std::string> label_{std::nullopt};
    std::optional<uint32> labelNum_{std::nullopt};

private:
    friend Section;
    Entry(
        std::string name,
        std::optional<std::string> value,
        std::optional<std::string> label,
        std::optional<uint32> labelNum
    );
};

struct PCONF_EXPORT Section : public Entry {
    [[nodiscard]] static SectionPtr create(
        std::string name,
        std::optional<std::string> label = std::nullopt,
        std::optional<uint32> labelNum = std::nullopt,
        Data entries = {}
    );

    Data entries_;

private:
    Section(
        std::string name, 
        std::optional<std::string> label,
        std::optional<uint32> labelNum,
        Data entries
   );
};

} // namespace pconf

