#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <variant>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>

class Configuration
{
public:
  static void outputConfig();
  static void updateBladesConfig();
  static void readConfig(wxWindow*);


  struct bladeConfig {
    std::string type{"NeoPixel (RGB)"};

    std::string dataPin{"Pin 1"};
    std::string colorType{"GRB"};
    int32_t numPixels{0};
    bool useRGBWithWhite{false};

    std::string Cree1{"<None>"};
    std::string Cree2{"<None>"};
    std::string Cree3{"<None>"};
    std::string Cree4{"<None>"};
    int32_t Cree1Resistance{0};
    int32_t Cree2Resistance{0};
    int32_t Cree3Resistance{0};
    int32_t Cree4Resistance{0};

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
  Configuration(const Configuration&) = delete;

  static ProffieBoard parseBoardType(const std::string&);
  static SaberProp parsePropSel(const std::string&);

  static void outputConfigTop(std::ofstream&);
  static void outputConfigTopDefaults(std::ofstream&);
  static void outputConfigTopGeneral(std::ofstream&);
  static void outputConfigTopPropSpecific(std::ofstream&);
  static void outputConfigTopSA22C(std::ofstream&);
  static void outputConfigTopFett263(std::ofstream&);
  static void outputConfigTopBC(std::ofstream&);
  static void outputConfigTopCaiwyn(std::ofstream&);
  static void outputConfigProp(std::ofstream&);
  static void outputConfigPresets(std::ofstream&);
  static void outputConfigPresetsStyles(std::ofstream&);
  static void outputConfigPresetsBlades(std::ofstream&);
  static void genWS281X(std::ofstream&, const Configuration::bladeConfig&);
  static void outputConfigButtons(std::ofstream&);

  static void readConfigTop(std::ifstream&);
  static void readDefine(std::string&);
  static void readConfigProp(std::ifstream&);
  static void readConfigPresets(std::ifstream&);
  static void readPresetArray(std::ifstream&);
  static void readBladeArray(std::ifstream&);
};
