#include "array.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/array.cpp
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

#include "config/config.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

using namespace config::presets;

Array::Array(Config& config) :
    Model(config),
    name_(root()),
    presets_(root()) {
    static const auto presetTable{[] {
        data::hier::Vector::RecvTable table;
        table.onInsert_ = data::map(&Array::onPresetInsert);
        return table;
    }()};
    amend(presets_, presetTable);

    static const auto nameTable{[] {
        data::hier::String::RecvTable table;
        table.onChange_ = data::map(&Array::onNameChange);
        return table;
    }()};
    amend(name_, nameTable);

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);

    recomputeIssues();
}

Array::~Array() = default;

auto Array::children() -> std::vector<Model *> {
    return {
        &name_,
        &presets_,
    };
}

const data::base::Integer& Array::issues() const {
    return mIssues;
}

void Array::recomputeIssues() {
    auto& config{root<Config>()};

    int32 issues{0};

    auto name{data::context(name_)};
    auto vec{data::context(config.presetArrays_)};

    for (const auto& child : vec.children()) {
        if (child.get() == this) continue;

        auto otherName{data::context(dynamic_cast<Array&>(*child).name_)};

        if (otherName.val() == name.val()) {
            issues |= eIssue_Name_Duplicate;
            break;
        }
    }

    if (name.val().empty()) {
        issues |= eIssue_Name_Empty;
    }

    mIssues.set(issues);
}

void Array::onPresetInsert(size) {
    root<Config>().syncStyles();
}

void Array::onNameChange() {
    recomputeIssues();
}

