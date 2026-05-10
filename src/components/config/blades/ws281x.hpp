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

#include "data/hierarchic/model.hpp"
#include "data/hierarchic/models/number.hpp"
#include "data/hierarchic/models/string.hpp"
#include "data/hierarchic/models/choice.hpp"
#include "data/hierarchic/models/bool.hpp"
#include "data/hierarchic/models/selection.hpp"
#include "data/hierarchic/models/vector.hpp"
#include "data/hierarchic/models/exclusive.hpp"
#include "utils/types.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace blades {

struct Blade;

struct CONFIG_EXPORT WS281X : data::hier::Model, data::Receiver {
    struct Split;

    WS281X(Blade&);

    std::vector<Model *> children() override;

    data::hier::Integer length_;

    data::hier::String dataPin_;

    data::hier::Choice colorOrder3_;
    data::hier::Choice colorOrder4_;
    data::hier::Bool hasWhite_;
    data::hier::Bool useRgbWithWhite_;

    data::hier::Selection powerPins_;

    data::hier::Vector splits_;

    Blade& parent_;

private:
    void onLength();
    void onHasWhiteSet();
    void onSplitsModify(size);
};

struct CONFIG_EXPORT WS281X::Split : data::hier::Model, data::Receiver {
    Split(WS281X&);

    std::vector<Model *> children() override;

    enum Type {
        eStandard,
        eReverse,
        eStride,
        eZig_Zag,
        eList,
        eMax,
    };
    data::hier::Exclusive type_;

    data::hier::Integer start_;
    data::hier::Integer end_;
    data::hier::Integer length_;

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
    data::hier::Integer segments_;

    // For list
    data::hier::String list_;
    [[nodiscard]] std::vector<uint32> listValues();

    data::hier::Integer brightness_;

private:
    WS281X& mParent;

    void onType(uint32);
    void onStart();
    void onEnd();
    void onLength();
    void onSegments();

    static void lengthFilter(
        const data::base::Integer::ROContext&, int32&
    );
    static void listFilter(
        const data::base::String::ROContext&, std::string&, size&
    );
};

} // namespace blades

} // namespace config

