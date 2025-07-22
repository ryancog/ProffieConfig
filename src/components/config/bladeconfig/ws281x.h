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
#include "ui/controls/toggle.h"
#include "ui/controls/radios.h"

#include "utils/types.h"

namespace Config {

struct Split {
    Split();

    PCUI::NumericData length;

    enum Type {
        STANDARD,
        REVERSE,
        STRIDE,
        ZIG_ZAG,
        TYPE_MAX
        // Blissfully ignorant of list
    };
    PCUI::RadiosData type;

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
};

struct WS281XBlade {
    WS281XBlade();

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
    };
    PCUI::ChoiceData colorOrder4;
    PCUI::ToggleData hasWhite;
    PCUI::ToggleData useRGBWithWhite;

    PCUI::CheckListData powerPins;

    vector<Split> splits;
};

} // namespace Config

