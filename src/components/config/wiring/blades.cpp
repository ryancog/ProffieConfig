#include "blades.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/blades.cpp
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

#include <config/wiring/components.h>
#include <config/wiring/computils.h>
#include <led/led.h>
#include <utils/image.h>

#include "wiring.h"

using namespace Config::Wiring::Components;
using namespace Config::Wiring;

std::optional<ColorOrderData> Config::Wiring::strToColorOrder(const string& str) {
    ColorOrderData ret;

    auto testStr{str};
    if (str.find("Color8::")) testStr = testStr.substr(strlen("Color8::"));
    if (testStr.length() != 3 and testStr.length() != 4) return std::nullopt;
    if (testStr.find('W') != string::npos) ret.useRGBWithWhite = true; // default is false
    for (char& chr : testStr) chr = static_cast<char>(std::toupper(chr));

    if (testStr == "BGR") ret.order = ColorOrder::BGR;
    else if (testStr == "BRG") ret.order = ColorOrder::BRG;
    else if (testStr == "GBR") ret.order = ColorOrder::GBR;
    else if (testStr == "GRB") ret.order = ColorOrder::GRB;
    else if (testStr == "RBG") ret.order = ColorOrder::RBG;
    else if (testStr == "RGB") ret.order = ColorOrder::RGB;
    else if (testStr == "BGRW") ret.order = ColorOrder::BGRW;
    else if (testStr == "BRGW") ret.order = ColorOrder::BRGW;
    else if (testStr == "GBRW") ret.order = ColorOrder::GBRW;
    else if (testStr == "GRBW") ret.order = ColorOrder::GRBW;
    else if (testStr == "RBGW") ret.order = ColorOrder::RBGW;
    else if (testStr == "RGBW") ret.order = ColorOrder::RGBW;
    else if (testStr == "WBGR") ret.order = ColorOrder::WBGR;
    else if (testStr == "WBRG") ret.order = ColorOrder::WBRG;
    else if (testStr == "WGBR") ret.order = ColorOrder::WGBR;
    else if (testStr == "WGRB") ret.order = ColorOrder::WGRB;
    else if (testStr == "WRBG") ret.order = ColorOrder::WRBG;
    else if (testStr == "WRGB") ret.order = ColorOrder::WRGB;
    else return std::nullopt;

    return ret;
}

void Blade::setBrightness(float32 brightness) { mBrightness = std::clamp<float32>(brightness, 0, 100); }

[[nodiscard]] float32 Blade::getBrightness() const { return mBrightness; }

BladeBase::AddressibleBase::AddressibleBase(ComponentClass type) : Component(type) {}

void BladeBase::AddressibleBase::init() {
    setType(Type::NORMAL);
    setSegmentLength(1);
}

BladeBase::AddressibleBase::AddressibleBlade::AddressibleBlade(bool reversed) : mReversed(reversed) {
    mPixels.resize(144);
}

BladeBase::AddressibleBase::Type BladeBase::AddressibleBase::getType() const { return mType; }
int32 BladeBase::AddressibleBase::AddressibleBlade::getNumLeds() const { return static_cast<int32>(mPixels.size()); }
bool BladeBase::AddressibleBase::AddressibleBlade::setLed(int32 idx, LED::Color16 color) { 
    if (idx > mPixels.size() - 1 or idx < 0) return false;

    if (mReversed) mPixels[mPixels.size() - 1 - idx] = color;
    else mPixels[idx] = color;

    return true;
}
vector<LED::Color16> BladeBase::AddressibleBase::AddressibleBlade::getColors() const { return mPixels; }

int32 BladeBase::AddressibleBase::calcTotalLength() const {
    int32 ret{0};
    for (const auto& blade : mBladeBase) {
        ret += static_cast<int32>(blade.mPixels.size());
    }
    return ret;
}
int32 BladeBase::AddressibleBase::getSegmentLength() const {
    if (mBladeBase.size() == 0) return 0;
    return static_cast<int32>(mBladeBase[0].mPixels.size());
}
int32 BladeBase::AddressibleBase::getNumSegments() const {
    return static_cast<int32>(mBladeBase.size());
}
float32 BladeBase::AddressibleBase::getBrightness(int32 segment) const {
    if (segment < 0 or segment > getNumSegments() - 1) return -1;

    return mBladeBase[segment].getBrightness();
}

void BladeBase::AddressibleBase::setType(Type type) {
    switch (type) {
        case Type::REVERSE:
            mBladeBase[0].mReversed = true;
        case Type::NORMAL:
            mBladeBase.resize(1);
            break;
        case Type::WITH_STRIDE:
        case Type::ZIG_ZAG:
            if (getNumSegments() < 2) setNumSegments(2);
            break;
    }
    mType = type;
}

bool BladeBase::AddressibleBase::setNumSegments(int32 numSegments) {
    if (mType != Type::ZIG_ZAG and mType != Type::WITH_STRIDE) return false;
    if (numSegments < 2) return false;

    auto currentSize{mBladeBase.size()};
    if (currentSize == numSegments) return true;

    mBladeBase.resize(numSegments);
    if (numSegments < currentSize) return true;

    for (auto idx{0}; idx < mBladeBase.size(); idx++) {
        auto& blade{mBladeBase[idx]};

        blade.mPixels.resize(getSegmentLength());
        blade.mReversed = (mType == Type::ZIG_ZAG) and ((idx % 2) == 1);
    }

    invalidateIcon();
    updatePadPositions();
    return true;
}

bool BladeBase::AddressibleBase::setSegmentLength(int32 segLength) {
    if (segLength < 1) return false;
    if (segLength == getSegmentLength()) return true;

    for (auto& blade : mBladeBase) {
        blade.mPixels.resize(segLength);
    }

    invalidateIcon();
    updatePadPositions();
    return true;
}

bool BladeBase::AddressibleBase::setBrightness(float32 brightness, int32 segment) {
    if (segment < 0 or segment >= getNumSegments()) return false;

    mBladeBase[segment].setBrightness(brightness);
    return true;
}

std::unique_ptr<BladeBase::AddressibleBase::AddressibleBlade> BladeBase::AddressibleBase::getBlade(int32 segment) {
    if (segment < 1 or segment >= getNumSegments()) return nullptr;

    return std::make_unique<AddressibleBlade>(mBladeBase[segment]);
}

BladeBase::SimpleBase::SimpleBase(int32 numLeds, ComponentClass type) : Component(type) {
    mLedIDs.resize(numLeds);
}

BladeBase::SimpleBase::SimpleBlade::SimpleBlade(vector<LED::Data> leds) : mLeds(std::move(leds)) {}

int32 BladeBase::SimpleBase::SimpleBlade::getNumLeds() const { return 1; };
bool BladeBase::SimpleBase::SimpleBlade::setLed(int32 led, LED::Color16 color) { 
    if (led != 0) return false; 
    
    mColor.zero();
    for (const auto& led : mLeds) {
        mColor += led.calculateColor(color);
        mColor.normalize();
    }
    return true;
}

void BladeBase::SimpleBase::setBrightness(float32 brightness) { mBrightness = std::clamp<float32>(brightness, 0, 100); }
[[nodiscard]] float32 BladeBase::SimpleBase::getBrightness() const { return mBrightness; }

vector<LED::Color16> BladeBase::SimpleBase::SimpleBlade::getColors() const { return { mColor }; }

std::unique_ptr<BladeBase::SimpleBase::SimpleBlade> BladeBase::SimpleBase::generateBlade() const {
    vector<LED::Data> bladeLeds;
    bladeLeds.reserve(mLedIDs.size());
    for (const auto& ledID : mLedIDs) {
        auto ledRecordIt{LED::getRecords().find(ledID)};
        if (ledRecordIt == LED::getRecords().end()) continue; // This probably shouldn't fail silently
        
        bladeLeds.push_back(ledRecordIt->second->constructData());
    }
    
    auto ret{std::make_unique<SimpleBlade>(std::move(bladeLeds))};
    ret->setBrightness(mBrightness);
    return ret;
}

vector<LED::ID> BladeBase::SimpleBase::getLedIDs() const { return mLedIDs; }
bool BladeBase::SimpleBase::setLedID(int32 led, LED::ID id) {
    if (led < 0 or led >= mLedIDs.size()) return false;

    mLedIDs[led] = id;
    return true;
}


WS281X::WS281X() : Cloneable(ComponentClass::WS281X_STRIP) {
    init();
    Pad::create(*this, "-", true);
    Pad::create(*this, "-");
    Pad::create(*this, "+", true);
    Pad::create(*this, "+");
    Pad::create(*this, "DIN", true);
    Pad::create(*this, "DOUT");

    setPadTypeGenerator(NEG_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG, PadType::BATT_NEG>>);
    setPadTypeGenerator(NEG_OUT, [](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};

        PadTypesSet ret{parentComp.getPad(NEG_IN)->calcDeepTypes()};
        ret.emit.addPad({parentComp.getID(), pad.getID()});
        return ret;
    });
    setPadTypeGenerator(POS_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(POS_OUT, [](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};

        PadTypesSet ret{parentComp.getPad(POS_IN)->calcDeepTypes()};
        ret.emit.addPad({parentComp.getID(), pad.getID()});
        return ret;
    });
    setPadTypeGenerator(DATA_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::NPXL_DATA>>);
    setPadTypeGenerator(DATA_OUT, staticTypesGen<StaticEmittedTypes<PadType::NPXL_DATA>>);
}

void WS281X::doSetPadPositions() {
    auto size{getUnorientedSize()};
    switch (getType()) {
        case AddressibleBase::Type::NORMAL:
        case AddressibleBase::Type::REVERSE:
            setPadPosition(NEG_IN, {0, 1});
            setPadPosition(DATA_IN, {0, 3});
            setPadPosition(POS_IN, {0, 5});
            setPadPosition(NEG_OUT, {size.x, 1});
            setPadPosition(DATA_OUT, {size.x, 3});
            setPadPosition(POS_OUT, {size.x, 5});
            break;
        case AddressibleBase::Type::WITH_STRIDE:
        case AddressibleBase::Type::ZIG_ZAG:
            break;
    }
}

wxBitmap WS281X::doGetIcon() const {
    auto icon{Image::loadPNG("components/pixel")};
    auto iconSize{icon.GetDIPSize()};
    auto ret{Image::newBitmap({iconSize.x * getSegmentLength(), iconSize.y})};
    {
        wxMemoryDC retDC{ret};
        for (auto idx{0}; idx < getSegmentLength(); idx++) {
            retDC.DrawBitmap(icon, {idx * iconSize.x, 0});
        }
    }
    return std::move(ret);
}

string WS281X::getHumanName() const { return HUMAN_NAME; }

SPI::SPI() : Cloneable(ComponentClass::SPI_STRIP) {
    init();
    Pad::create(*this, "-", true);
    Pad::create(*this, "-");
    Pad::create(*this, "+", true);
    Pad::create(*this, "+");
    Pad::create(*this, "DIN", true);
    Pad::create(*this, "DOUT");
    Pad::create(*this, "CLK", true);
    Pad::create(*this, "CLK");

    setPadTypeGenerator(NEG_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG, PadType::BATT_NEG>>);
    setPadTypeGenerator(NEG_OUT, [](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};
        const auto& parentWiring{parentComp.getParent()};

        PadTypesSet ret{pad};
        if (not parentWiring.getNetsConnectedToPad({parentComp.getID(), NEG_IN}).empty()) {
            ret.absorb(parentComp.getPad(NEG_IN)->calcDeepTypes());
        }
        return ret;
    });
    setPadTypeGenerator(POS_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(POS_OUT, [](const Pad& pad) -> PadTypesSet {
        const auto& parentComp{pad.getParent()};
        const auto& parentWiring{parentComp.getParent()};

        PadTypesSet ret{pad};
        if (not parentWiring.getNetsConnectedToPad({parentComp.getID(), POS_IN}).empty()) {
            ret.absorb(parentComp.getPad(POS_IN)->calcDeepTypes());
        }
        return ret;
    });
    setPadTypeGenerator(DATA_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BITBANG_DATA>>);
    setPadTypeGenerator(DATA_OUT, staticTypesGen<StaticEmittedTypes<PadType::BITBANG_DATA>>);
    setPadTypeGenerator(CLK_IN, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BITBANG_CLK>>);
    setPadTypeGenerator(CLK_OUT, staticTypesGen<StaticEmittedTypes<PadType::BITBANG_CLK>>);
}

void SPI::doSetPadPositions() {
    switch(getType()) {
        case AddressibleBase::Type::NORMAL:
        case AddressibleBase::Type::REVERSE:
        case AddressibleBase::Type::WITH_STRIDE:
        case AddressibleBase::Type::ZIG_ZAG:
            break;
    }
}

wxBitmap SPI::doGetIcon() const {

}

string SPI::getHumanName() const { return HUMAN_NAME; }

TriStar::TriStar() : Cloneable(3, ComponentClass::TRISTAR) {
    Pad::create(*this, "LED1 -", true);
    Pad::create(*this, "LED1 +", true);
    Pad::create(*this, "LED2 -", true);
    Pad::create(*this, "LED2 +", true);
    Pad::create(*this, "LED3 -", true);
    Pad::create(*this, "LED3 +", true);

    setPadTypeGenerator(LED1_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED1_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(LED2_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED2_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(LED3_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED3_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
}

void TriStar::doSetPadPositions() {
}
wxBitmap TriStar::doGetIcon() const { /* temp garbage value */ return { wxSize(200, 200), 32 }; }
string TriStar::getHumanName() const { return HUMAN_NAME; }

QuadStar::QuadStar() : Cloneable(4, ComponentClass::QUADSTAR) {
    Pad::create(*this, "LED1 -", true);
    Pad::create(*this, "LED1 +", true);
    Pad::create(*this, "LED2 -", true);
    Pad::create(*this, "LED2 +", true);
    Pad::create(*this, "LED3 -", true);
    Pad::create(*this, "LED3 +", true);
    Pad::create(*this, "LED4 -", true);
    Pad::create(*this, "LED4 +", true);

    setPadTypeGenerator(LED1_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED1_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(LED2_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED2_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(LED3_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED3_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(LED4_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(LED4_POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
}

void QuadStar::doSetPadPositions() {
}
wxBitmap QuadStar::doGetIcon() const { /* temp garbage value */ return { wxSize(200, 200), 32 }; }
string QuadStar::getHumanName() const { return HUMAN_NAME; }

SingleLED::SingleLED() : Cloneable(1, ComponentClass::SINGLE_DIODE) {
    Pad::create(*this, "-", true);
    Pad::create(*this, "+", true);

    // TODO: logic to allow either CONTROLLED_NEG or CONTROLLED_POS
    setPadTypeGenerator(NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
    setPadTypeGenerator(POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
}

void SingleLED::doSetPadPositions() {
    setPadPosition(POS, {0, 1});
    setPadPosition(NEG, {1, 2});
}
wxBitmap SingleLED::doGetIcon() const { return Image::loadPNG("components/led"); }
string SingleLED::getHumanName() const { return HUMAN_NAME; }

String::String() : Cloneable(ComponentClass::STRING_BLADE) {
    Pad::create(*this, "+", true);
    Pad::create(*this, "SEG1 -", true);
    Pad::create(*this, "SEG2 -", true);
    Pad::create(*this, "SEG3 -", true);
    Pad::create(*this, "SEG4 -", true);
    Pad::create(*this, "SEG5 -", true);
    Pad::create(*this, "SEG6 -", true);
    Pad::create(*this, "Clash -");

    using namespace Components;

    auto segmentTypeGenerator{[](const Pad& pad) -> PadTypesSet {
        PadID pbPad{};
        auto pbClass{pad.getParent().getParent().getProffieboardID().type.compClass};
        switch (pad.getID()) {
            case SEG1_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED1, ProffieBoardV2::LED1, ProffieBoardV3::LED1);
                break;
            case SEG2_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED2, ProffieBoardV2::LED2, ProffieBoardV3::LED2);
                break;
            case SEG3_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED3, ProffieBoardV2::LED3, ProffieBoardV3::LED3);
                break;
            case SEG4_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED4, ProffieBoardV2::LED4, ProffieBoardV3::LED4);
                break;
            case SEG5_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED5, ProffieBoardV2::LED5, ProffieBoardV3::LED5);
                break;
            case SEG6_NEG:
                pbPad = switchBoard<PadID>(pbClass, ProffieBoardV1::LED6, ProffieBoardV2::LED6, ProffieBoardV3::LED6);
                break;
        }
        PadTypesSet ret{pad};
        ret.receive.addPad({pad.getParent().getParent().getProffieboardID(), pbPad});
        return ret;
    }};

    setPadTypeGenerator(POS, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::BATT_POS>>);
    setPadTypeGenerator(SEG1_NEG, segmentTypeGenerator);
    setPadTypeGenerator(SEG2_NEG, segmentTypeGenerator);
    setPadTypeGenerator(SEG3_NEG, segmentTypeGenerator);
    setPadTypeGenerator(SEG4_NEG, segmentTypeGenerator);
    setPadTypeGenerator(SEG5_NEG, segmentTypeGenerator);
    setPadTypeGenerator(SEG6_NEG, segmentTypeGenerator);
    setPadTypeGenerator(CLASH_NEG, staticTypesGen<StaticEmittedTypes<>, StaticReceivedTypes<PadType::CONTROLLED_NEG>>);
}

void String::doSetPadPositions() {
}
wxBitmap String::doGetIcon() const { /* temp garbage value */ return { wxSize(200, 200), 32 }; }
string String::getHumanName() const { return HUMAN_NAME; }
void String::setBrightness(float32 brightness) { mBrightness = std::clamp<float32>(brightness, 0, 100); }
float32 String::getBrightness() const { return mBrightness; }

String::StringBlade::StringBlade(LED::Data mainLED, std::optional<LED::Data> clashLED) : 
    mainLED(mainLED), 
    clashLED(clashLED) {}

int32 String::StringBlade::getNumLeds() const { return 6; }
bool String::StringBlade::setLed(int32 led, LED::Color16 color) {
    if (led < 0 or led >= 6) return false;

    if (led == 0 and clashLED) mClash = clashLED->calculateColor(color);
    mLEDs[led] = mainLED.calculateColor(color);
    return true;
}
vector<LED::Color16> String::StringBlade::getColors() const {
    vector<LED::Color16> ret;
    ret.reserve(mLEDs.size());
    for (const auto& color : mLEDs) {
        ret.push_back(color);
        ret.back() += mClash;
        ret.back().normalize();
    }
    return ret;
}

std::unique_ptr<String::StringBlade> String::generateBlade() const {
    LED::Data mainData;
    auto mainLEDRecordIt{LED::getRecords().find(mainLED)};
    if (mainLEDRecordIt != LED::getRecords().end()) mainData = mainLEDRecordIt->second->constructData();

    std::optional<LED::Data> clashData;
    auto clashLEDRecordIt{LED::getRecords().find(clashLED)};
    if (clashLEDRecordIt != LED::getRecords().end()) clashData = clashLEDRecordIt->second->constructData();

    auto ret{std::make_unique<StringBlade>(mainData, clashData)};
    ret->setBrightness(mBrightness);
    return ret;
}


