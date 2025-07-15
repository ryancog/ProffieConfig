#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/blades.h
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

#include <optional>

#include <led/color.h>
#include <led/led.h>
#include <styles/emulation.h>
#include <utils/types.h>

#include "../private/export.h"
#include "wiring.h"

namespace Config::Wiring {

enum class ColorOrder {
    BGR,
    BRG,
    GBR,
    GRB,
    RBG,
    RGB,
    BGRW,
    BRGW,
    GBRW,
    GRBW,
    RBGW,
    RGBW,
    WBGR,
    WBRG,
    WGBR,
    WGRB,
    WRBG,
    WRGB,
};

struct ColorOrderData {
    ColorOrder order{ColorOrder::GRB};
    bool useRGBWithWhite{false};
};

std::optional<ColorOrderData> strToColorOrder(const string&);

struct Blade {
    virtual ~Blade() = default;

    [[nodiscard]] virtual int32 getNumLeds() const = 0;
    // Resize the vector and inserts the color (if valid index), after any needed
    // blade-specific calculations.
    // TODO: Wrap the virtual in a caller so that brightness is applied blade-agnostically.
    virtual bool setLed(int32 idx, LED::Color16) = 0; 
    // Return all colors after augmentation.
    [[nodiscard]] virtual vector<LED::Color16> getColors() const = 0;

    void setBrightness(float32);
    [[nodiscard]] float32 getBrightness() const;

private:
    float32 mBrightness{100};
};


} // namespace Config::Wiring

namespace Config::Wiring::Components::BladeBase {
// A base for WS281X and Spi-type blades.
struct CONFIG_EXPORT AddressibleBase : Component {
    ColorOrderData colorOrder;
    bool useRGBWithWhite{false};

    // Both `WITH_STRIDE` and `ZIG_ZAG` must have at least two segments.
    enum class Type {
        // Single strip or chained "SubBlade"
        NORMAL,
        // Simple invert during `setLed()`
        REVERSE,
        // Stride is number of `blades`. Length is equal for all `blades`.
        WITH_STRIDE,
        /*
         * Stride is similarly number of blades, rows calculated and equal for all `blades`.
         * If we had something like this:
         *         |-->--|
         * ----- ----- -----
         * | X | | X | | X | 1 <-- Index
         * | X | | X | | X | 2
         * | X | | X | | X | 3
         * | X | | X | | X | 4
         * ----- ----- -----
         *   |-->--|
         *
         *
         * Then there would be 4 columns and 3 rows. The virtualized
         * "columns" are running perpendicular to the physical strips.
         */
        ZIG_ZAG,
    };
    struct AddressibleBlade : public Blade {
        // Set defaults like vector size.
        AddressibleBlade(bool reversed = false);

        [[nodiscard]] int32 getNumLeds() const override;
        bool setLed(int32, LED::Color16) override;
        [[nodiscard]] vector<LED::Color16> getColors() const override;

    private:
        friend class AddressibleBase;
        vector<LED::Color16> mPixels;
        bool mReversed;
    };

    [[nodiscard]] Type getType() const;
    // Calculated from all `mBladeBase`
    [[nodiscard]] int32 calcTotalLength() const;
    // From segment. All segments equal.
    [[nodiscard]] int32 getSegmentLength() const;
    // Effectively `mBladeBase.size()`
    [[nodiscard]] int32 getNumSegments() const;
    // Return brightness of segment. -1 if invalid segment num.
    [[nodiscard]] float32 getBrightness(int32 segment = 0) const; 
    // Update type and update necessary parameters. (i.e. Resizing `mBladeBase` to ensure sanity)
    void setType(Type);
    // Return false if type doesn't allow. (e.g. Setting too few blades with ZZ, or too many with Normal)
    bool setNumSegments(int32);
    // Return false if invalid length (i.e Length <= 0)
    bool setSegmentLength(int32);
    // Return false if invalid segment or value, true otherwise and update brightness
    bool setBrightness(float32 brightness, int32 segment = 0);

    // If within the valid range, return a new representative blade.
    std::unique_ptr<AddressibleBlade> getBlade(int32 segment = 0);

protected:
    AddressibleBase(ComponentClass);
    // This MUST BE CALLED by implementing objects
    // These things can't be called in the ctor because virtual methods need to be setup first.
    void init();

private:
    Type mType{AddressibleBase::Type::NORMAL};

    // For a SubBlade w/ Stride or SubBladeZZ where there's multiple
    // segments per physical strip.
    //
    // This is different from (and represented as such) multiple strips which are simply
    // chained and use normal SubBlade or SubBladeReverse.
    //
    // Normally only one, but a vector in the case of WithStride and ZZ
    vector<AddressibleBlade> mBladeBase;
};
// Used by all "dumb" LED components
struct CONFIG_EXPORT SimpleBase : Component {
    struct SimpleBlade : public Blade {
        SimpleBlade(vector<LED::Data> leds);

        [[nodiscard]] int32 getNumLeds() const override;
        bool setLed(int32, LED::Color16) override; 
        [[nodiscard]] vector<LED::Color16> getColors() const override;

        void configLeds(vector<string>);
        [[nodiscard]] vector<string> getLedConfig() const;

    private:
        vector<LED::Data> mLeds;
        LED::Color16 mColor;
    };

    // Generate a new representative blade
    [[nodiscard]] std::unique_ptr<SimpleBlade> generateBlade() const;

    [[nodiscard]] vector<LED::ID> getLedIDs() const;

    // Sets led at idx to id.
    // Returns false if `led` is OOB. Does not check `id` for validity.
    bool setLedID(int32 led, LED::ID id);

    void setBrightness(float32);
    [[nodiscard]] float32 getBrightness() const;

protected:
    SimpleBase(int32 numLeds, ComponentClass);

private:
    float32 mBrightness{100};
    vector<LED::ID> mLedIDs;
};

} // namespace Config::Wiring::Components::BladeBase
namespace Config::Wiring::Components {

struct CONFIG_EXPORT WS281X : Cloneable<BladeBase::AddressibleBase, WS281X> {
    WS281X();

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"WS281X LED Strip"};

    enum {
        NEG_IN,
        NEG_OUT,
        POS_IN,
        POS_OUT,
        DATA_IN,
        DATA_OUT,
    };
};

struct CONFIG_EXPORT SPI : Cloneable<BladeBase::AddressibleBase, SPI> {
    SPI();

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"SPI LED Strip"};

    enum {
        NEG_IN,
        NEG_OUT,
        POS_IN,
        POS_OUT,
        DATA_IN,
        DATA_OUT,
        CLK_IN,
        CLK_OUT,
    };
};


struct CONFIG_EXPORT TriStar : Cloneable<BladeBase::SimpleBase, TriStar> {
    TriStar();

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Tri-Star LED"};

    enum {
        LED1_NEG,
        LED1_POS,
        LED2_NEG,
        LED2_POS,
        LED3_NEG,
        LED3_POS,
    };
};

struct CONFIG_EXPORT QuadStar : Cloneable<BladeBase::SimpleBase, QuadStar> {
    QuadStar();

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"Quad-Star LED"};

    enum {
        LED1_NEG,
        LED1_POS,
        LED2_NEG,
        LED2_POS,
        LED3_NEG,
        LED3_POS,
        LED4_NEG,
        LED4_POS,
    };
};

struct CONFIG_EXPORT SingleLED : Cloneable<BladeBase::SimpleBase, SingleLED> {
    SingleLED();

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    static constexpr cstring HUMAN_NAME{"LED"};

    enum {
        NEG,
        POS,
    };
};

// Savi-style string blade
struct CONFIG_EXPORT String : Cloneable<Component, String> { 
    String();

    struct StringBlade : Blade {
        StringBlade(LED::Data mainLED, std::optional<LED::Data> clashLED = std::nullopt);
        // 6 LEDs. If clash is used, it uses the first led style output.
        [[nodiscard]] int32 getNumLeds() const override;
        bool setLed(int32, LED::Color16) override; 
        [[nodiscard]] vector<LED::Color16> getColors() const override;

        LED::Data mainLED;
        std::optional<LED::Data> clashLED;

    private:
        vector<LED::Color16> mLEDs;
        LED::Color16 mClash; // Mixed with leds during `getColors()`
    };
    [[nodiscard]] std::unique_ptr<StringBlade> generateBlade() const;

    LED::ID mainLED{LED::BuiltIn::NO_LED.first};
    LED::ID clashLED{LED::BuiltIn::NO_LED.first};

    void doSetPadPositions() override;
    [[nodiscard]] wxBitmap doGetIcon() const override;
    [[nodiscard]] string getHumanName() const override;

    void setBrightness(float32);
    [[nodiscard]] float32 getBrightness() const;

    static constexpr cstring HUMAN_NAME{"LED String Blade"};

    enum {
        POS,
        SEG1_NEG,
        SEG2_NEG,
        SEG3_NEG,
        SEG4_NEG,
        SEG5_NEG,
        SEG6_NEG,
        CLASH_NEG,
    };

private:
    float32 mBrightness{100};
};

} // namespace Config::Wiring::Components

