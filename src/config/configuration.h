// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "pages/bladespage.h"

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

  bool outputConfig();
  bool outputConfig(const std::string&);
  bool exportConfig();
  void readConfig();
  void readConfig(const std::string&);
  void importConfig();

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

  ProffieBoard parseBoardType(const std::string&);
  SaberProp parsePropSel(const std::string&);

  uint32_t numBlades{0};

private:
  bool runPrechecks();

  void outputConfigTop(std::ofstream&);
  void outputConfigTopGeneral(std::ofstream&);
  void outputConfigTopBladeAwareness(std::ofstream& configOutput);
  void outputConfigTopPropSpecific(std::ofstream&);
  void outputConfigTopSA22C(std::ofstream&);
  void outputConfigTopFett263(std::ofstream&);
  void outputConfigTopBC(std::ofstream&);
  void outputConfigTopCaiwyn(std::ofstream&);
  void outputConfigProp(std::ofstream&);
  void outputConfigPresets(std::ofstream&);
  void outputConfigPresetsStyles(std::ofstream&);
  void outputConfigPresetsBlades(std::ofstream&);
  void genWS281X(std::ofstream&, const BladesPage::BladeConfig&);
  void outputConfigButtons(std::ofstream&);

  void readConfigTop(std::ifstream&);
  void readConfigProp(std::ifstream&);
  void readConfigPresets(std::ifstream&);
  void readConfigStyles(std::ifstream&);
  void replaceStyles(const std::string&, const std::string&);
  void readPresetArray(std::ifstream&);
  void readBladeArray(std::ifstream&);
};
