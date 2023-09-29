#include <vector>
#include <string>
#include <fstream>

#pragma once

class Configuration
{
public:
  static void outputConfig();
  static void updateConfig();

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
    BC,
    CAIWYN
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

    bool noLockupHold{false};

    bool disableguestureNoBlade{false};
    // Stab On
    bool stabOn{false};
    bool stabOnFast{false};
    bool stabOnPreon{false};
    bool stabOnNoBattle{false};
    // Swing On
    bool swingOn{false};
    uint16_t swingOnSpeed;
    bool swingOnFast{false};
    bool swingOnPreon{false};
    bool swingOnNoBattle{false};
    // Twist On
    bool twistOn{false};
    bool twistOnFast{false};
    bool twistOnPreon{false};
    bool twistOnNoBattle{false};
    // Thrust On
    bool thrustOn{false};
    bool thrustOnFast{false};
    bool thrustOnPreon{false};
    bool thrustOnNoBattle{false};
    // Twist Off
    bool twistOff{false};
    bool twistOffFast{false};
    bool twistOffPostoff{false};

    // Battle Mode
    bool guestureEnBattle{false};
    uint16_t lockupDelay;
    bool battleModeToggle{false};
    bool battleModeAlways{false};
    bool battleModeOnStart{false};
    bool battleModeDisablePWR{false};
    uint16_t battleModeClash;

    // Force Push
    bool forcePush{false};
    bool forcePushBM{false};
    uint16_t forcePushLength;

    // Edit Mode/Settings
    bool editEnable{false};
    bool editMode{false};
    bool editSettings{false};

    // Quote Player
    bool enableQuotePlayer{false};
    bool randomizeQuotePlayer{false};
    bool forcePlayerDefault{false};
    bool quotePlayerDefault{false};

    bool pwrClash{false};
    bool pwrLockup{false};
    bool pwrHoldOff{false};
    bool auxHoldLockup{false};
    bool meltguestureAlways{false};
    bool volumeCircular{false};
    bool brightnessCircular{false};
    bool pwrWakeguesture{false};
    bool noExtraEffects{false};
    bool specialAbilities{false};
    bool multiPhase{false};
    bool spinMode{false};
    bool saveGuesture{false};
    bool saveChoreo{false};
    bool dualModeSound{false};
    bool clashStrengthSound{false};
    uint16_t clashStrengthSoundMaxClash;
    bool quickPresetSelect{false};
    bool spokenColors{false};
    bool spokenBatteryNone{false};
    bool spokenBatteryVolts{false};
    bool spokenBatteryPercent{false};

    bool beepErrors{false};
    bool trackPlayerPrompts{false};
    bool fontChangeOTF{false};
    bool styleChangeOTF{false};
    bool presetCopyOTF{false};
    bool battleToggle{false};
    bool multiBlast{false};
    bool multiBlastDisableToggle{false};
    bool multiBlastSwing{false};
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

  static void updateGeneralConfig();
  static void updatePropConfig();
  static void updatePresetsConfig();
  static void updateBladesConfig();
  static void updateHardwareConfig();

  static void outputConfigTop(std::ofstream&);
  static void outputConfigTopDefaults(std::ofstream&);
  static void outputConfigTopGeneral(std::ofstream&);
  static void outputConfigTopPropSpecific(std::ofstream&);
  static void outputConfigTopSA22C(std::ofstream&);

  static void outputConfigProp(std::ofstream&);

  static void outputConfigPresets(std::ofstream&);
  static void outputConfigPresetsStyles(std::ofstream&);
  static void outputConfigPresetsBlades(std::ofstream&);
  static void genWS281X(std::ofstream&, Configuration::bladeConfig);

  static void outputConfigButtons(std::ofstream&);
};
