#include "components.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/components.cpp
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

#include <wx/dcmemory.h>

#include <config/wiring/computils.h>
#include <utils/image.h>

#include "wiring.h"

using namespace Config::Wiring::Components;
namespace Config::Wiring::Components {

} // namespace Config::Wiring::Components

Resistor::Resistor() : Cloneable(ComponentClass::RESISTOR) {
    Pad::create(*this, "", true);
    Pad::create(*this, "", true);

    auto typesGenerator{[](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};
        const auto& parentWiring{parentComp.getParent()};
        PadID otherPad{};
        switch (pad.getID()) {
            case 0: otherPad = 1; break;
            case 1: otherPad = 0; break;
        }

        PadTypesSet ret{pad};
        if (parentWiring.getNetsConnectedToPad({parentComp.getID(), otherPad}).empty()) {
            ret.absorb(staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<
                    PadType::BATT_NEG,
                    PadType::CONTROLLED_NEG,
                    PadType::BATT_POS,
                    PadType::CONTROLLED_POS,
                    PadType::POW_3V3,
                    PadType::POW_5V,
                    PadType::SD_POWER
                    >>(pad));
        } else {
            ret.absorb(parentComp.getPad(otherPad)->calcDeepTypes());
        }
        return ret;
    }};

    setPadTypeGenerator(0, typesGenerator);
    setPadTypeGenerator(1, typesGenerator);
}

void Resistor::doSetPadPositions() {
    setPadPosition(0, {0, 1});
    setPadPosition(1, {5, 1});
}

wxBitmap Resistor::doGetIcon() const { return Image::loadPNG("components/resistor"); }
string Resistor::getHumanName() const { return HUMAN_NAME; }

void Resistor::setResistance(uint32 resistance) { 
    if (resistance < 10) resistance = 10;
    // Add an upper limit?
    mResistance = resistance;
}
uint32 Resistor::getResistance() const { return mResistance; }

ButtonBase::ButtonBase(ComponentClass type) : Component(type) {
    Pad::create(*this, {}, true);
    Pad::create(*this, {}, true);

    auto typesGenerator{[](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};
        const auto& parentWiring{parentComp.getParent()};
        PadID otherPad{};
        switch (pad.getID()) {
            case 0: otherPad = 1; break;
            case 1: otherPad = 0; break;
        }

        constexpr auto SIG_TYPES{StaticEmittedTypes<PadType::GPIO, PadType::BUTTON>::EMIT_TYPES};
        constexpr auto GND_TYPES{StaticEmittedTypes<PadType::BATT_NEG>::EMIT_TYPES};
        PadTypesSet ret{pad};
        if (not parentWiring.getNetsConnectedToPad({parentComp.getID(), otherPad}).empty()) {
            auto otherTypes{parentComp.getPad(otherPad)->calcDeepTypes()};
            bool sigConn{false};
            for (auto type : SIG_TYPES) {
                sigConn = otherTypes.emit.types.contains(type);
                if (sigConn) break;
            }

            if (sigConn) {}

            bool gndConn{false};
            for (auto type : GND_TYPES) {
                gndConn = otherTypes.emit.types.contains(type);
                if (gndConn) break;
            }


            auto ret{staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<>>(pad)};
        }
        return ret;
    }};
    setPadTypeGenerator(0, typesGenerator);
    setPadTypeGenerator(1, typesGenerator);
}

MomentaryButton::MomentaryButton() : Cloneable(ComponentClass::MOMENTARY_BUTTON) {}

void MomentaryButton::doSetPadPositions() {}

wxBitmap MomentaryButton::doGetIcon() const { return Image::loadPNG("components/momentary-button"); }

string MomentaryButton::getHumanName() const { return HUMAN_NAME; }

LatchingButton::LatchingButton() : Cloneable(ComponentClass::LATCHING_BUTTON) {}

void LatchingButton::doSetPadPositions() {}

wxBitmap LatchingButton::doGetIcon() const { return Image::loadPNG("components/latching-button"); }

string LatchingButton::getHumanName() const { return HUMAN_NAME; }

TouchButton::TouchButton() : Cloneable(ComponentClass::TOUCH_BUTTON) {}

void TouchButton::doSetPadPositions() {}

wxBitmap TouchButton::doGetIcon() const { return Image::loadPNG("components/touch-button"); }

string TouchButton::getHumanName() const { return HUMAN_NAME; }

RFID::RFID() : Cloneable(ComponentClass::RFID) {
    Pad::create(*this, "VCC", true);
    Pad::create(*this, "GND", true);
    Pad::create(*this, "TX", true);

    setPadTypeGenerator(VCC, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS, PadType::POW_5V, PadType::POW_3V3>>);
    setPadTypeGenerator(GND, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_NEG, PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(TX, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::SER_RX>>);
}

void RFID::doSetPadPositions() {
    setPadPosition(VCC, {1, 1});
    setPadPosition(GND, {1, 3});
    setPadPosition(TX, {1, 5});
}
wxBitmap RFID::doGetIcon() const { return Image::loadPNG("components/rfid"); }
string RFID::getHumanName() const { return HUMAN_NAME; }

bool RFID::addCommand(Command::ID id, const string& cmd, const string& arg) {
    if (mCommands.find(id) != mCommands.end()) return false;

    mCommands.emplace(id, Command{ id, cmd, arg });
    return true;
}

bool RFID::removeCommand(Command::ID id) {
    auto commandIt{mCommands.find(id)};
    if (commandIt == mCommands.end()) return false;

    mCommands.erase(commandIt);
    return true;
}

ProffieBoardV1::ProffieBoardV1() : Cloneable(ComponentClass::PROFFIEBOARD_V1) {
    Pad::create(*this, "LED1");
    Pad::create(*this, "LED2");
    Pad::create(*this, "LED3");
    Pad::create(*this, "LED4");
    Pad::create(*this, "LED5");
    Pad::create(*this, "LED6");
    Pad::create(*this, "Button1");
    Pad::create(*this, "Button2");
    Pad::create(*this, "Button3");
    Pad::create(*this, "Data1");
    Pad::create(*this, "Data2");
    Pad::create(*this, "Data3");
    Pad::create(*this, "Data4");
    Pad::create(*this, "Data5");
    Pad::create(*this, "TX");
    Pad::create(*this, "RX");
    Pad::create(*this, "SDA");
    Pad::create(*this, "SCL");
    Pad::create(*this, "GND1");
    Pad::create(*this, "GND2");
    Pad::create(*this, "BATT_NEG1");
    Pad::create(*this, "BATT_NEG2");
    Pad::create(*this, "BATT_POS");
    Pad::create(*this, "POWER_5V");
    Pad::create(*this, "POWER_3V3");
    Pad::create(*this, "SPKR_NEG");
    Pad::create(*this, "SPKR_POS");
    Pad::create(*this, "Reset");
    Pad::create(*this, "SWDIO");
    Pad::create(*this, "SWDCLK");

    // TODO: All this stuff I was lazy about.
    // setPadTypeFunc(TX, []() -> PadTypeSet { return {.types{PadType::SER_TX}}; });
    // setPadTypeFunc(RX, []() -> PadTypeSet { return {.types{PadType::SER_RX}}; });
    // setPadTypeFunc(SDA, []() -> PadTypeSet { return {.types{PadType::I2C_SDA}}; });
    // setPadTypeFunc(SCL, []() -> PadTypeSet { return {.types{PadType::I2C_SCL}}; });
    // setPadTypeFunc(LED1, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED2, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED3, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED4, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED5, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED6, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(Reset, []() -> PadTypeSet { return {}; });
    // setPadTypeFunc(SWDIO, []() -> PadTypeSet { return {}; });
    // setPadTypeFunc(SWDCLK, []() -> PadTypeSet { return {}; });
}

void ProffieBoardV1::doSetPadPositions() {

}

wxBitmap ProffieBoardV1::doGetIcon() const { return Image::loadPNG("components/proffieboardv1"); }

string ProffieBoardV1::getHumanName() const { return HUMAN_NAME; }

ProffieBoardV2::ProffieBoardV2() : Cloneable(ComponentClass::PROFFIEBOARD_V2) {
    Pad::create(*this, "LED1");    
    Pad::create(*this, "LED2");    
    Pad::create(*this, "LED3");    
    Pad::create(*this, "LED4");    
    Pad::create(*this, "LED5");    
    Pad::create(*this, "LED6");    
    Pad::create(*this, "Button1");    
    Pad::create(*this, "Button2");    
    Pad::create(*this, "Button3");    
    Pad::create(*this, "Data1");    
    Pad::create(*this, "Data2");    
    Pad::create(*this, "Data3");    
    Pad::create(*this, "Data4");    
    Pad::create(*this, "TX");    
    Pad::create(*this, "RX");    
    Pad::create(*this, "SDA");    
    Pad::create(*this, "SCL");    
    Pad::create(*this, "GND1");    
    Pad::create(*this, "GND2");    
    Pad::create(*this, "BATT_NEG1");    
    Pad::create(*this, "BATT_NEG2");    
    Pad::create(*this, "BATT_POS");    
    Pad::create(*this, "POWER_5V");    
    Pad::create(*this, "POWER_3V3");    
    Pad::create(*this, "SD_POWER");    
    Pad::create(*this, "SPKR_POS");    
    Pad::create(*this, "SPKR_NEG");    
    Pad::create(*this, "Reset");    
    Pad::create(*this, "SWDIO");    
    Pad::create(*this, "SWDCLK");    

    // TODO: All this stuff I was lazy about
    // setPadTypeFunc(Button1, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Button2, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Button3, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Data1, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Data2, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Data3, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(Data4, []() -> PadTypeSet { return {.types{PadType::GPIO}}; });
    // setPadTypeFunc(GND1, []() -> PadTypeSet { return {.types{PadType::BATT_NEG}}; });
    // setPadTypeFunc(GND2, []() -> PadTypeSet { return {.types{PadType::BATT_NEG}}; });
    // setPadTypeFunc(BATT_NEG1, []() -> PadTypeSet { return {.types{PadType::BATT_NEG}}; });
    // setPadTypeFunc(BATT_NEG2, []() -> PadTypeSet { return {.types{PadType::BATT_NEG}}; });
    // setPadTypeFunc(BATT_POS, []() -> PadTypeSet { return {.types{PadType::BATT_POS}}; });
    // setPadTypeFunc(POWER_5V, []() -> PadTypeSet { return {.types{PadType::POW_5V}}; });
    // setPadTypeFunc(POWER_3V3, []() -> PadTypeSet { return {.types{PadType::POW_3V3}}; });
    // setPadTypeFunc(SD_POWER, []() -> PadTypeSet { return {.types{PadType::SD_POWER}}; });
    // setPadTypeFunc(SPKR_POS, []() -> PadTypeSet { return {.types{PadType::SPEAKER}}; });
    // setPadTypeFunc(SPKR_NEG, []() -> PadTypeSet { return {.types{PadType::SPEAKER}}; });
    // setPadTypeFunc(TX, []() -> PadTypeSet { return {.types{PadType::SER_TX}}; });
    // setPadTypeFunc(RX, []() -> PadTypeSet { return {.types{PadType::SER_RX}}; });
    // setPadTypeFunc(SDA, []() -> PadTypeSet { return {.types{PadType::I2C_SDA}}; });
    // setPadTypeFunc(SCL, []() -> PadTypeSet { return {.types{PadType::I2C_SCL}}; });
    // setPadTypeFunc(LED1, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED2, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED3, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED4, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED5, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(LED6, []() -> PadTypeSet { return {.types{PadType::CONTROLLED_NEG}}; });
    // setPadTypeFunc(Reset, []() -> PadTypeSet { return {}; });
    // setPadTypeFunc(SWDIO, []() -> PadTypeSet { return {}; });
    // setPadTypeFunc(SWDCLK, []() -> PadTypeSet { return {}; });
}

void ProffieBoardV2::doSetPadPositions() {

}

wxBitmap ProffieBoardV2::doGetIcon() const { return Image::loadPNG("components/proffieboardv2"); }

string ProffieBoardV2::getHumanName() const { return HUMAN_NAME; }

ProffieBoardV3::ProffieBoardV3() : Cloneable(ComponentClass::PROFFIEBOARD_V3) {
    Pad::create(*this, "LED1");    
    Pad::create(*this, "LED2");    
    Pad::create(*this, "LED3");    
    Pad::create(*this, "LED4");    
    Pad::create(*this, "LED5");    
    Pad::create(*this, "LED6");    
    Pad::create(*this, "Button1");    
    Pad::create(*this, "Button2");    
    Pad::create(*this, "Button3");    
    Pad::create(*this, "Data1");    
    Pad::create(*this, "Data2");    
    Pad::create(*this, "Data3");    
    Pad::create(*this, "Data4");    
    Pad::create(*this, "Free1");    
    Pad::create(*this, "Free2");    
    Pad::create(*this, "Free3");    
    Pad::create(*this, "TX");    
    Pad::create(*this, "RX");    
    Pad::create(*this, "SDA");    
    Pad::create(*this, "SCL");    
    Pad::create(*this, "GND1");    
    Pad::create(*this, "GND2");    
    Pad::create(*this, "BATT_NEG1");    
    Pad::create(*this, "BATT_NEG2");    
    Pad::create(*this, "BATT_POS");    
    Pad::create(*this, "POWER_5V");    
    Pad::create(*this, "POWER_3V3");    
    Pad::create(*this, "SD_POWER");    
    Pad::create(*this, "SPKR_POS");    
    Pad::create(*this, "SPKR_NEG");    

    setPadTypeGenerator(LED1, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED2, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED3, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED4, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED5, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED6, staticTypesGen<StaticEmittedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(Button1, staticTypesGen<StaticEmittedTypes<PadType::GPIO, PadType::BUTTON>>);
    setPadTypeGenerator(Button2, staticTypesGen<StaticEmittedTypes<PadType::GPIO, PadType::BUTTON>>);
    setPadTypeGenerator(Button3, staticTypesGen<StaticEmittedTypes<PadType::GPIO, PadType::BUTTON>>);
    setPadTypeGenerator(Data1, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Data2, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Data3, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Data4, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Free1, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Free2, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(Free3, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(TX, staticTypesGen<StaticEmittedTypes<PadType::SER_TX>>);
    setPadTypeGenerator(RX, staticTypesGen<StaticEmittedTypes<PadType::SER_RX>>);
    setPadTypeGenerator(SDA, staticTypesGen<StaticEmittedTypes<PadType::I2C_SDA>>);
    setPadTypeGenerator(SCL, staticTypesGen<StaticEmittedTypes<PadType::I2C_SCL>>);
    setPadTypeGenerator(GND1, staticTypesGen<StaticEmittedTypes<PadType::BATT_NEG>>);
    setPadTypeGenerator(GND2, staticTypesGen<StaticEmittedTypes<PadType::BATT_NEG>>);
    setPadTypeGenerator(BATT_NEG1, staticTypesGen<StaticEmittedTypes<PadType::BATT_NEG>>);
    setPadTypeGenerator(BATT_NEG2, staticTypesGen<StaticEmittedTypes<PadType::BATT_NEG>>);
    setPadTypeGenerator(BATT_POS, staticTypesGen<StaticEmittedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(POWER_5V, staticTypesGen<StaticEmittedTypes<PadType::POW_5V>>);
    setPadTypeGenerator(POWER_3V3, staticTypesGen<StaticEmittedTypes<PadType::POW_3V3>>);
    setPadTypeGenerator(SD_POWER, staticTypesGen<StaticEmittedTypes<PadType::SD_POWER>>);
    setPadTypeGenerator(SPKR_POS, staticTypesGen<StaticEmittedTypes<PadType::SPEAKER_POS>>);
    setPadTypeGenerator(SPKR_NEG, staticTypesGen<StaticEmittedTypes<PadType::SPEAKER_NEG>>);
}

void ProffieBoardV3::doSetPadPositions() {
    setPadPosition(Button3,     Point{3, 1});
    setPadPosition(Free2,       Point{5, 1});
    setPadPosition(Free3,       Point{7, 1});
    setPadPosition(Data1,       Point{9, 1});
    setPadPosition(Data4,       Point{11, 1});
    setPadPosition(RX,          Point{13, 1});
    setPadPosition(TX,          Point{15, 1});
    setPadPosition(POWER_5V,    Point{17, 1});
    setPadPosition(BATT_POS,    Point{19, 1});
    setPadPosition(GND1,        Point{21, 1});
    setPadPosition(BATT_NEG1,   Point{23, 1});
    setPadPosition(LED1,        Point{25, 1});
    setPadPosition(Button2,     Point{27, 1});
    setPadPosition(LED2,        Point{27, 3});
    setPadPosition(LED3,        Point{27, 5});
    setPadPosition(LED4,        Point{27, 9});
    setPadPosition(LED5,        Point{27, 11});
    setPadPosition(Button1,     Point{27, 13});
    setPadPosition(LED6,        Point{25, 13});
    setPadPosition(BATT_NEG2,   Point{23, 13});
    setPadPosition(SPKR_POS,    Point{21, 13});
    setPadPosition(SPKR_NEG,    Point{19, 13});
    setPadPosition(SDA,         Point{17, 13});
    setPadPosition(SCL,         Point{15, 13});
    setPadPosition(Data2,       Point{13, 13});
    setPadPosition(Free1,       Point{11, 13});
    setPadPosition(Data3,       Point{9, 13});
    setPadPosition(GND2,        Point{7, 13});
    setPadPosition(POWER_3V3,   Point{5, 13});
    setPadPosition(SD_POWER,    Point{3, 13});
}

wxBitmap ProffieBoardV3::doGetIcon() const { 
    auto boardBMP{Image::loadPNG("components/proffieboardv3")};
    return boardBMP;
}

string ProffieBoardV3::getHumanName() const { return HUMAN_NAME; }

