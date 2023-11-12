#pragma once

#include "bladespage.h"

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

  std::vector<Configuration::presetConfig> presets;

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
  void genWS281X(std::ofstream&, const BladesPage::bladeConfig&);
  void outputConfigButtons(std::ofstream&);

  void readConfigTop(std::ifstream&);
  void readDefine(std::string&);
  void readConfigProp(std::ifstream&);
  void readConfigPresets(std::ifstream&);
  void readConfigStyles(std::ifstream&);
  void replaceStyles(const std::string&, const std::string&);
  void readPresetArray(std::ifstream&);
  void readBladeArray(std::ifstream&);
};
