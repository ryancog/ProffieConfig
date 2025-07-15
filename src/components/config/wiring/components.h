#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/components.h
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

#include <utils/types.h>

#include "wiring.h"
#include "../private/export.h"

namespace Config::Wiring::Components {

struct CONFIG_EXPORT Resistor : Cloneable<Component, Resistor> {
    Resistor();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    void setResistance(uint32);
    [[nodiscard]] uint32 getResistance() const;

    static constexpr cstring HUMAN_NAME{"Resistor"};

private:
    // in mOhms
    uint32 mResistance{1000};
};

// My Post here: https://crucible.hubbe.net/t/button-types/5137
// goes over the different button types.
//
// For now I'm going to be blissfully ignorant of `PotClass` and `Rotary` I think...
// Afaict the explanation means that `FastButton` and `FloatingButton` are basically
// irrelevant and can be ignored for now.
//
// So the relevant button types are:
// - `Button`: Momentary; Triggers when signal goes low (GND to GPIO short)
// - `PullDownButton`: Momentary; Triggers when signal goes high (+ to GPIO short)
// - `LatchingButton`: Latching (duh); Triggered when signal is low
// - `InvertedLatchingButton`: Latching (dug); Triggered when signal is high
// - `TouchButton`: Single wire connected to a Button pad (specifically, doesn't work with normal GPIO)

// Used in order to have common pad type logic for the "normal" button
// types. Currently only `TouchButton` is an outlier and does not use this.
// 
// Has two legs.
struct CONFIG_EXPORT ButtonBase : Component {
    ButtonBase(ComponentClass);
};

struct CONFIG_EXPORT MomentaryButton : Cloneable<ButtonBase, MomentaryButton> {
    MomentaryButton();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Momentary Button"};
};

struct CONFIG_EXPORT LatchingButton : Cloneable<ButtonBase, LatchingButton> {
    LatchingButton();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Latching Button"};
};

struct CONFIG_EXPORT TouchButton : Cloneable<ButtonBase, TouchButton> {
    TouchButton();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Touch Button"};
};

struct CONFIG_EXPORT RFID : Cloneable<Component, RFID> {
    RFID();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    struct Command {
        using ID = uint64;
        ID id;
        string cmd;
        string arg;
    };

    bool addCommand(Command::ID, const string& cmd, const string& arg);
    bool removeCommand(Command::ID);

    static constexpr cstring HUMAN_NAME{"RFID"};

    enum {
        VCC,
        GND,
        TX,
    };

private:
    std::unordered_map<Command::ID, Command> mCommands;
};

struct CONFIG_EXPORT ProffieBoardV1 : Cloneable<Component, ProffieBoardV1> {
    ProffieBoardV1();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Proffieboard V1"};

    // Order must be synced with ctor pad order.
    // Used by other components for named pad IDs.
    enum {
        LED1,
        LED2,
        LED3,
        LED4,
        LED5,
        LED6,
        Button1,
        Button2,
        Button3,
        Data1,
        Data2,
        Data3,
        Data4,
        Data5,
        TX,
        RX,
        SDA,
        SCL,
        GND1,
        GND2,
        BATT_NEG1,
        BATT_NEG2,
        BATT_POS,
        POWER_5V,
        POWER_3V3,
        SPKR_NEG,
        SPKR_POS,
        Reset,
        SWDIO,
        SWDCLK,
    };
};

struct CONFIG_EXPORT ProffieBoardV2 : Cloneable<Component, ProffieBoardV2> {
    ProffieBoardV2();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Proffieboard V2"};

    enum {
        LED1,
        LED2,
        LED3,
        LED4,
        LED5,
        LED6,
        Button1,
        Button2,
        Button3,
        Data1,
        Data2,
        Data3,
        Data4,
        TX,
        RX,
        SDA,
        SCL,
        GND1,
        GND2,
        BATT_NEG1,
        BATT_NEG2,
        BATT_POS,
        POWER_5V,
        POWER_3V3,
        SD_POWER,
        SPKR_POS,
        SPKR_NEG,
        Reset,
        SWDIO,
        SWDCLK,
    };
};

struct CONFIG_EXPORT ProffieBoardV3 : Cloneable<Component, ProffieBoardV3> {
    ProffieBoardV3();
    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Proffieboard V3"};

    enum {
        LED1,
        LED2,
        LED3,
        LED4,
        LED5,
        LED6,
        Button1,
        Button2,
        Button3,
        Data1,
        Data2,
        Data3,
        Data4,
        Free1,
        Free2,
        Free3,
        TX,
        RX,
        SDA,
        SCL,
        GND1,
        GND2,
        BATT_NEG1,
        BATT_NEG2,
        BATT_POS,
        POWER_5V,
        POWER_3V3,
        SD_POWER,
        SPKR_POS,
        SPKR_NEG,
    };
};

} // namespace Config::Wiring::Components


