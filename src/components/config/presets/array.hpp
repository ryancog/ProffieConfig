#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/array.hpp
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

#include "data/number.hpp"
#include "data/string.hpp"
#include "data/vector.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace presets {

struct CONFIG_EXPORT Array : data::Node {
    Array(data::Node *);
    ~Array() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    enum {
        eIssue_Name_Empty       = 1 << 0,
        eIssue_Name_Duplicate   = 1 << 1,
    };

    [[nodiscard]] const data::Integer& issues() const;

    data::String name_;
    data::Vector presets_;

private:
    void recomputeIssues();

    data::Integer mIssues;
};

} // namespace presets

} // namespace config

