#include "define.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/define.cpp
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

#include "utils/string.hpp"

namespace {

constexpr std::string_view NAME_STR{"Name"};
constexpr std::string_view VALUE_STR{"Value"};

} // namespace

using namespace config::settings;

Define::Define(
    data::Node *parent,
    std::string&& name,
    std::string&& value
) : data::Node(parent) {
    CreationScope createScope(*this);

    const auto defineFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
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
    name_.setFilter(defineFilter);
    data::String::Context{name_}.change(std::move(name), 0);
    data::String::Context{value_}.change(std::move(value), 0);
}

Define::~Define() = default;

bool Define::enumerate(const EnumFunc& func) {
    if (func(name_, strID(NAME_STR), NAME_STR)) return true;
    if (func(value_, strID(VALUE_STR), VALUE_STR)) return true;
    return false;
}

data::Model *Define::find(uint64 id) {
    if (id == strID(NAME_STR)) return &name_;
    if (id == strID(VALUE_STR)) return &value_;
    return nullptr;
}

