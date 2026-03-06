#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/bladeawareness.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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
#include "data/choice.hpp"
#include "data/number.hpp"
#include "data/selection.hpp"
#include "data/string.hpp"
#include "data/hierarchy/node.hpp"

#include "config_export.h"

namespace config {

struct Settings;

namespace settings {

struct CONFIG_EXPORT BladeAwareness : data::Node {
    BladeAwareness(Settings&);
    ~BladeAwareness() override;

    bool enumerate(const EnumFunc&) override;
    [[nodiscard]] Model *find(uint64) override;

    struct {
        data::Bool enable_;
        data::String pin_;
    } bladeDetect_;

    struct {
        data::Bool enable_;
        data::String pin_;

        data::Choice mode_;
        data::String bridgePin_;
        data::Integer pullup_;

        data::Bool powerForId_;
        data::Selection powerPins_;

        data::Bool continuousScanning_;
        data::Integer continuousInterval_;
        data::Integer continuousTimes_;
    } bladeId_;
};

} // namespace settings

} // namespace config

