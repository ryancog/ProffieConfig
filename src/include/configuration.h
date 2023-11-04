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
  Configuration();
  static Configuration* instance;

  void outputConfig();
  void outputConfig(const std::string&);
  void exportConfig();
  void readConfig();
  void readConfig(const std::string&);
  void importConfig();

  void updateBladesConfig();

  struct bladeConfig {
    std::string type{"WS281X (RGB)"};

    std::string dataPin{"bladePin"};
    std::string colorType{"GRB"};
    int32_t numPixels{0};
    bool useRGBWithWhite{false};

    std::string Star1{"<None>"};
    std::string Star2{"<None>"};
    std::string Star3{"<None>"};
    std::string Star4{"<None>"};
    int32_t Star1Resistance{0};
    int32_t Star2Resistance{0};
    int32_t Star3Resistance{0};
    int32_t Star4Resistance{0};

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
    PIXEL_3,
    PIXEL_4,
    STAR_3,
    STAR_4,
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
  enum class STARTYPE {
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

  std::vector<Configuration::presetConfig> presets;
  std::vector<Configuration::bladeConfig> blades;

private:
  ProffieBoard parseBoardType(const std::string&);
  SaberProp parsePropSel(const std::string&);

  void outputConfigTop(std::ofstream&);
  void outputConfigTopDefaults(std::ofstream&);
  void outputConfigTopGeneral(std::ofstream&);
  void outputConfigTopPropSpecific(std::ofstream&);
  void outputConfigTopSA22C(std::ofstream&);
  void outputConfigTopFett263(std::ofstream&);
  void outputConfigTopBC(std::ofstream&);
  void outputConfigTopCaiwyn(std::ofstream&);
  void outputConfigProp(std::ofstream&);
  void outputConfigPresets(std::ofstream&);
  void outputConfigPresetsStyles(std::ofstream&);
  void outputConfigPresetsBlades(std::ofstream&);
  void genWS281X(std::ofstream&, const Configuration::bladeConfig&);
  void outputConfigButtons(std::ofstream&);

  void readConfigTop(std::ifstream&);
  void readDefine(std::string&);
  void readConfigProp(std::ifstream&);
  void readConfigPresets(std::ifstream&);
  void readPresetArray(std::ifstream&);
  void readBladeArray(std::ifstream&);
};
