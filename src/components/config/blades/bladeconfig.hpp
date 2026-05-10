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
#include "data/hierarchic/model.hpp"
#include "data/hierarchic/models/number.hpp"
#include "data/hierarchic/models/selector.hpp"
#include "data/hierarchic/models/string.hpp"
#include "data/primitive/models/bool.hpp"
#include "data/primitive/models/number.hpp"
#include "utils/types.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace blades {

constexpr int32 NO_BLADE{1000000000};

struct CONFIG_EXPORT BladeConfig : data::hier::Model, data::Receiver {
    BladeConfig(Config&);

    void onActivate() override;
    std::vector<Model *> children() override;

    data::hier::Vector blades_;

    enum Issues {
        eIssue_None = 0,
        eIssue_No_Preset_Array  = 1UL << 0,
        eIssue_Duplicate_ID     = 1UL << 1,
        eIssue_Duplicate_Name   = 1UL << 2,

        eIssue_Mask             = 0b111,
        eIssue_Errors           =
            eIssue_Duplicate_Name |
            eIssue_No_Preset_Array,
        eIssue_Warnings         = eIssue_Duplicate_ID,
    };

    [[nodiscard]] const data::base::Integer& issues() const;

    data::hier::String name_;
    data::hier::Selector presetArray_;
    data::hier::Integer id_;
    data::prim::Bool noBladeId_;

private:
    void recomputeIssues();

    void onNameChange();
    void onID();
    void onNoBladeIDSet();
    void onPresetArrayChoice();
    void onBladesModify(size);

    data::prim::Integer mIssues;
};

struct CONFIG_EXPORT Blade : data::hier::Model {
    Blade(Config&);

    std::vector<Model *> children() override;

    enum Type {
        // NOLINTNEXTLINE(readability-identifier-naming)
        eWS281X,
        eSimple,
        eUnassigned,
    };

    const data::base::Selector& type();
    const data::base::Vector& types();

    [[nodiscard]] WS281X& ws281x() LIFETIMEBOUND;
    [[nodiscard]] Simple& simple() LIFETIMEBOUND;

    data::hier::Integer brightness_;

private:
    data::hier::Selector mType;
    data::hier::Vector mTypes;
};

} // namespace blades

} // namespace config

