#include <vector>
#include <string>
#include <fstream>

#pragma once

class Configuration
{
public:
  static void outputConfig();
  static void updateBladesConfig();


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

  enum class SaberProp {
    DEFAULT,
    SA22C,
    FETT263,
    SHTOK,
    BC,
    CAIWYN
  };
  enum class ProffieBoard {
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

  static std::vector<Configuration::presetConfig> presets;
  static std::vector<Configuration::bladeConfig> blades;


private:
  Configuration();
  Configuration(const Configuration& obj) = delete;

  static ProffieBoard parseBoardType(const std::string&);
  static SaberProp parsePropSel(const std::string&);

  static void outputConfigTop(std::ofstream&);
  static void outputConfigTopDefaults(std::ofstream&);
  static void outputConfigTopGeneral(std::ofstream&);
  static void outputConfigTopPropSpecific(std::ofstream&);
  static void outputConfigTopSA22C(std::ofstream&);

  static void outputConfigProp(std::ofstream&);

  static void outputConfigPresets(std::ofstream&);
  static void outputConfigPresetsStyles(std::ofstream&);
  static void outputConfigPresetsBlades(std::ofstream&);
  static void genWS281X(std::ofstream&, const Configuration::bladeConfig&);

  static void outputConfigButtons(std::ofstream&);
};
