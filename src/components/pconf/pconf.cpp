#include "pconf.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/pconf/private/pconf.cpp
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

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>

#include <log/branch.h>
#include <log/logger.h>

namespace PConf {

} // namespace PConf

PConf::Entry::Entry(
        string name, 
        std::optional<string> value,
        std::optional<string> label,
        std::optional<int32_t> labelNum
        ) : name(std::move(name)), value(std::move(value)), label(std::move(label)), labelNum(labelNum) {}

PConf::Section::Section(
        string name, 
        std::optional<string> label, 
        std::optional<int32_t> labelNum,
        Data entries
        ) : Entry(std::move(name), std::nullopt, std::move(label), labelNum), entries(std::move(entries)) {}
