#include <vector>
#include <string>
#include <fstream>

#pragma once

class Configuration
{
public:
    static void outputConfig();
    static void updateConfig();
    static void updateGeneralConfig();
    static void updatePresetsConfig();
    static void updateBladesConfig();
    static void updateHardwareConfig();

    struct bladeConfig {
        std::string type{"NeoPixel (RGB)"};

        std::string dataPin{"Pin 1"};
        std::string colorType{"GRB"};
        int32_t numPixels{144};
        bool useRGBWithWhite{false};

        std::string Cree1{"Red"};
        std::string Cree2{"Green"};
        std::string Cree3{"Blue"};
        std::string Cree4{"White"};
        int32_t Cree1Resistance{1000};
        int32_t Cree2Resistance{0};
        int32_t Cree3Resistance{240};
        int32_t Cree4Resistance{550};

        bool usePowerPin1{false};
        bool usePowerPin2{false};
        bool usePowerPin3{false};
        bool usePowerPin4{false};
        bool usePowerPin5{false};
        bool usePowerPin6{false};

        bool isSubBlade{false};
        bool subBladeWithStride{false};

        struct subBladeInfo {
            int32_t startPixel{0};
            int32_t endPixel{0};
        };
        std::vector<subBladeInfo> subBlades{};
    };
    struct presetConfig {
        std::vector<std::string> styles{};
        std::string name{""};
        std::string dirs{""};
        std::string track{""};
    };

    enum class SABERPROP {
        DEFAULT,
        SA22C,
        FETT263,
        SHTOK,
        BC
    };
    enum class PROFFIEBOARD {
        V1,
        V2,
        V3
    };
    enum class ORIENTATION {
        FETS_TOWARDS_BLADE,
        USB_TOWARDS_BLADE,
        TOP_TOWARDS_BLADE,
        BOTTOM_TOWARDS_BLADE,
        CUSTOM
    };
    enum class BLADETYPE {
        NEOPIXEL_3,
        NEOPIXEL_4,
        CREE_3,
        CREE_4,
        SINGLECOLOR,
        NONE
    };
    enum class C_ORDER {
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
        BGRw,
        BRGw,
        GBRw,
        GRBw,
        RBGw,
        RGBw,
        wBGR,
        wBRG,
        wGBR,
        wGRB,
        wRBG,
        wRGB
    };
    enum class CREETYPE {
        RED,
        GREEN,
        PCAMBER,
        AMBER,
        BLUE,
        REDORANGE,
        WHITE,
        XPL,
        NOLED
    };

    struct {
        bool audioEnabled{true};
        bool motionEnabled{true};
        bool ws2811Enabled{true};
        bool sdEnabled{true};
        bool disableBasicStyles{true};
    } static defaults;
    struct {
        PROFFIEBOARD board{PROFFIEBOARD::V2};
        bool massStorage{false};
        bool webUSB{false};
    } static boardConfig;
    struct {
        SABERPROP prop{SABERPROP::SA22C};
        bool stabOn{false};
        bool swingOn{false};
        bool twistOn{false};
        bool thrustOn{false};
        bool twistOff{false};
        bool battleMode{false};
        bool gestureBattle{false};
        bool revertLockup{false};
        bool forcePush{false};
        int32_t forcePushLength{5};
        int32_t lockupDelay{200};
    } static propConfig;
    struct {
        int32_t numButtons{2};
        int32_t volume{2000};
        double clashThreshold{3};

        int32_t maxLEDs{144};
    } static general;
    struct {
        int32_t saveVolume{true};
        int32_t savePreset{true};
        int32_t saveColorChange{true};
        int32_t disableColorChange{false};
    } static options;
    struct {
        bool hasOLED{false};
        bool hasBLE{false};
        bool hasRFID{false};
        bool bladeDetect{false};
        std::string bladeDetectPin{""};
    } static features;
    struct {
        int32_t idleTimout{10 * 60};
        int32_t pliTimeout{2 * 60};
        int32_t motionTimeout{15 * 60};
        bool devCommands{false};

        ORIENTATION orientation{ORIENTATION::FETS_TOWARDS_BLADE};
        struct {
            int32_t x{0};
            int32_t y{0};
            int32_t z{0};
        } orientationRotation;

        bool dualPowerButtons{false};
    } static tweaks;

    static std::vector<Configuration::presetConfig> presets;
    static std::vector<Configuration::bladeConfig> blades;

private:
    Configuration();
    Configuration(const Configuration& obj) = delete;
};
