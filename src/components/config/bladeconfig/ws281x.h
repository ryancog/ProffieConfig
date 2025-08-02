#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/ws281x.h
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

#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/numeric.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "ui/controls/radios.h"
#include "utils/types.h"

#include "../private/export.h"

namespace Config {

struct Config;

struct WS281XBlade;

struct CONFIG_EXPORT Split {
    Split(Config&, WS281XBlade&);

    enum Type {
        STANDARD,
        REVERSE,
        STRIDE,
        ZIG_ZAG,
        LIST,
        TYPE_MAX
    };
    PCUI::RadiosData type;

    PCUI::NumericData start;
    PCUI::NumericData end;
    PCUI::NumericData length;

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
    PCUI::NumericData segments;

    // For list
    PCUI::TextData list;

private:
    Config& mConfig;
    WS281XBlade& mParent;
};

struct CONFIG_EXPORT WS281XBlade {
    WS281XBlade(Config&);

    PCUI::NumericData length;

    PCUI::ComboBoxData dataPin;

    enum ColorOrder3 {
        GRB,
        GBR,
        BGR,
        BRG,
        RGB,
        RBG,
    };
    PCUI::ChoiceData colorOrder3;
    enum ColorOrder4 {
        GRBW,
        GBRW,
        BGRW,
        BRGW,
        RGBW,
        RBGW,

        WGRB,
        WGBR,
        WBGR,
        WBRG,
        WRGB,
        WRBG,

        ORDER4_WFIRST_START = WGRB,
        ORDER4_WFIRST_END = WRBG,
    };
    PCUI::ChoiceData colorOrder4;
    PCUI::ToggleData hasWhite;
    PCUI::ToggleData useRGBWithWhite;

    PCUI::CheckListData powerPins;

    PCUI::ChoiceData splitSelect;

    [[nodiscard]] const vector<std::unique_ptr<Split>>& splits() const { return mSplits; }
    [[nodiscard]] Split& split(uint32 idx) const {
        assert(idx < mSplits.size());
        return *mSplits[idx];
    }

    Split& addSplit();
    void removeSplit(uint32);

private:
    Config& mConfig;
    vector<std::unique_ptr<Split>> mSplits;
};

} // namespace Config

