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

struct CONFIG_EXPORT Blade {
    Blade(Config&);

    enum class Type {
        // NOLINTNEXTLINE(readability-identifier-naming)
        WS281X,
        Simple,
        Unassigned,
    };

    data::Selector type_;
    data::Vector types_;

    data::Integer brightness_;
};

struct CONFIG_EXPORT BladeConfig {
    BladeConfig(Config&);

    data::Vector blades_;

    // // Notify of issues
    // pcui::Notifier notifyData;

    // enum {
    //     ISSUE_NONE = 0,
    //     ISSUE_NO_PRESETARRAY  = 1UL << 0,
    //     ISSUE_DUPLICATE_ID    = 1UL << 1,
    //     ISSUE_DUPLICATE_NAME  = 1UL << 2,
    // };
    // static constexpr auto ISSUE_WARNINGS{
    //     ISSUE_DUPLICATE_ID
    // };
    // static constexpr auto ISSUE_ERRORS{
    //     ISSUE_DUPLICATE_NAME | ISSUE_NO_PRESETARRAY
    // };
    // [[nodiscard]] uint32 computeIssues() const;

    // /**
    //  * @param Issue or bitor'd Issue's
    //  * @return untranslated string
    //  */
    // [[nodiscard]] static string issueString(uint32 issues);

    data::String name_;
    data::Choice presetArray_;
    data::Integer id_;
    data::Bool noBladeId_;
};

} // namespace blades

} // namespace config

