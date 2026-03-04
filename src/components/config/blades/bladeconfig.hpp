#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/bladeconfig.hpp
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

#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "data/bool.hpp"
#include "data/number.hpp"
#include "data/selector.hpp"
#include "data/string.hpp"
#include "utils/types.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace blades {

constexpr int32 NO_BLADE{1000000000};

struct CONFIG_EXPORT BladeConfig : data::Node {
    BladeConfig(data::Node *);
    ~BladeConfig() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    data::Vector blades_;

    enum {
        eIssue_None = 0,
        eIssue_No_Preset_Array  = 1UL << 0,
        eIssue_Duplicate_ID     = 1UL << 1,
        eIssue_Duplicate_Name   = 1UL << 2,
        eIssue_Mask             = 0b111,
    };
    static constexpr auto ISSUE_WARNINGS{eIssue_Duplicate_ID};
    static constexpr auto ISSUE_ERRORS{
        eIssue_Duplicate_Name | eIssue_No_Preset_Array
    };

    // TODO: Another observable.
    data::Integer issues_;

    /**
     * @param Issue or bitor'd Issue's
     * @return untranslated string
     */
    [[nodiscard]] static std::string issueString(uint32 issues);

    data::String name_;
    data::Selector presetArray_;
    data::Integer id_;
    data::Bool noBladeId_;

private:
    void recomputeIssues();

};

struct CONFIG_EXPORT Blade : data::Node {
    Blade(data::Node *);
    ~Blade() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    [[nodiscard]] WS281X& ws281x() [[clang::lifetimebound]];
    [[nodiscard]] Simple& simple() [[clang::lifetimebound]];

    enum Type {
        // NOLINTNEXTLINE(readability-identifier-naming)
        eWS281X,
        eSimple,
        eUnassigned,
    };

    data::Selector type_;
    // TODO: Observable with malleable innards
    data::Vector types_;

    data::Integer brightness_;
};

} // namespace blades

} // namespace config

