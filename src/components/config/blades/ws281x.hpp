#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/ws281x.hpp
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
#include "data/choice.hpp"
#include "data/helpers/exclusive.hpp"
#include "data/number.hpp"
#include "data/selection.hpp"
#include "data/string.hpp"
#include "data/vector.hpp"
#include "utils/types.hpp"

#include "config_export.h"

namespace config::blades {

struct Blade;

struct CONFIG_EXPORT WS281X : data::Node {
    struct Split;

    WS281X(data::Node *);
    ~WS281X() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    data::Integer length_;

    data::String dataPin_;

    data::Choice colorOrder3_;
    data::Choice colorOrder4_;
    data::Bool hasWhite_;
    data::Bool useRgbWithWhite_;

    data::Selection powerPins_;

    data::Vector splits_;
};

struct CONFIG_EXPORT WS281X::Split : data::Node {
    Split(data::Node *);
    ~Split() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    enum Type {
        eStandard,
        eReverse,
        eStride,
        eZig_Zag,
        eList,
        eMax,
    };
    data::Exclusive<> type_;

    data::Integer start_;
    data::Integer end_;
    data::Integer length_;

    /*
     * Stride: Data goes like:
     * |---|   |---|   |---|
     * |   | ^ |   | ^ |   |
     * | | | | | | | | | | |
     * | | | | | | | | | | |
     * | | | | | | | | | | |
     * | | | | | | | | | | |
     * | V | | | V | | | V |
     * |   | | |   | | |   |
     * |---|   |---|   |---|
     *
     * But animation should go:
     *
     * --------------->
     * --------------->
     * --------------->
     *
     * ZigZag: Data goes like:
     *           ------>
     * |---|  |---|  |---|
     * |   |  |   |  |   |
     * | | |  | ^ |  | | |
     * | | |  | | |  | | |
     * | | |  | | |  | | |
     * | | |  | | |  | | |
     * | V |  | | |  | V |
     * |   |  |   |  |   |
     * |---|  |---|  |---|
     *    ------>
     *
     * But animation should go:
     *
     * --------------->
     * --------------->
     * --------------->        
     */

    // For stide and zigzag
    data::Integer segments_;

    // For list
    data::String list_;
    [[nodiscard]] std::vector<uint32> listValues();

    data::Integer brightness_;
};

} // namespace config::blades

