#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/bladeawareness.hpp
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

#include "data/hierarchic/model.hpp"
#include "data/hierarchic/models/bool.hpp"
#include "data/hierarchic/models/choice.hpp"
#include "data/hierarchic/models/number.hpp"
#include "data/hierarchic/models/selection.hpp"
#include "data/hierarchic/models/string.hpp"

#include "config_export.h"
#include "data/receiver.hpp"

namespace config {

struct Settings;

namespace settings {

struct CONFIG_EXPORT BladeAwareness : data::hier::Model, data::Receiver {
    BladeAwareness(Settings&);
    ~BladeAwareness() override;

    using Model::children;
    std::vector<const Model *> children() const override;

    struct {
        data::hier::Bool enable_;
        data::hier::String pin_;
    } bladeDetect_;

    struct {
        data::hier::Bool enable_;
        data::hier::String pin_;

        data::hier::Choice mode_;
        data::hier::String bridgePin_;
        data::hier::Integer pullup_;

        data::hier::Bool powerForId_;
        data::hier::Selection powerPins_;

        struct {
            data::hier::Bool enable_;
            data::hier::Integer interval_;
            data::hier::Integer times_;
        } continuous_;
    } bladeId_;

protected:
    void onActivate() override;

    void onDetectEnable();
    void onIDEnable();
    void onContinuousEnable();
    void onIDPower();
};

} // namespace settings

} // namespace config

