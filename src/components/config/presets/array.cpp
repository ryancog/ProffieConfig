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
#include "data/vector.hpp"
#include "utils/string.hpp"

using namespace config::presets;

namespace {

constexpr std::string_view NAME_STR{"Name"};
constexpr std::string_view PRESETS_STR{"Presets"};

} // namespace

Array::Array(data::Node *parent) :
    data::Node{parent},
    name_(this),
    presets_(this) {
    const auto nameFilter{[](std::string& str, size& pos) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};

    presets_.responder().onInsert_ = [](
        const data::Vector::ROContext& ctxt, size
    ) {
        ctxt.model().root<Config>()->syncStyles();
    };

    name_.responder().onChange_ = [](
        const data::String::ROContext& ctxt
    ) {
        ctxt.model().parent<Array>()->recomputeIssues();
    };

    recomputeIssues();
}

Array::~Array() = default;

bool Array::enumerate(const EnumFunc& func) {
    if (func(name_, strID(NAME_STR), NAME_STR)) return true;
    if (func(presets_, strID(PRESETS_STR), PRESETS_STR)) return true;
    return false;
}

data::Model *Array::find(uint64 id) {
    if (id == strID(NAME_STR)) return &name_;
    if (id == strID(PRESETS_STR)) return &presets_;
    return nullptr;
}

const data::Integer& Array::issues() const {
    return mIssues;
}

void Array::recomputeIssues() {
    auto& vec{*parent<data::Vector>()};

    int32 issues{0};

    data::String::ROContext nameCtxt{name_};

    data::Vector::ROContext vecCtxt{vec};
    for (const auto& child : vecCtxt.children()) {
        if (child.get() == this) continue;

        auto& otherName{static_cast<Array&>(*child).name_};
        data::String::ROContext otherNameCtxt{otherName};

        if (otherNameCtxt.val() == nameCtxt.val()) {
            issues |= eIssue_Name_Duplicate;
            break;
        }
    }

    if (nameCtxt.val().empty()) {
        issues |= eIssue_Name_Empty;
    }

    data::Integer::Context{mIssues}.set(issues);
}

