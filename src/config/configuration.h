// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "pages/bladespage.h"

#include <string>
#include <fstream>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>

class Configuration {
public:
  static bool outputConfig();
  static bool outputConfig(const std::string&);
  static bool exportConfig();
  static void readConfig();
  static void readConfig(const std::string&);
  static void importConfig();

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

  static ProffieBoard parseBoardType(const std::string&);

private:
  Configuration();
  Configuration(const Configuration&) = delete;

  static bool runPrechecks();

  static void outputConfigTop(std::ofstream&);
  static void outputConfigTopGeneral(std::ofstream&);
  static void outputConfigTopBladeAwareness(std::ofstream& configOutput);
  static void outputConfigTopPropSpecific(std::ofstream&);
  static void outputConfigTopSA22C(std::ofstream&);
  static void outputConfigTopFett263(std::ofstream&);
  static void outputConfigTopBC(std::ofstream&);
  static void outputConfigTopCaiwyn(std::ofstream&);
  static void outputConfigProp(std::ofstream&);
  static void outputConfigPresets(std::ofstream&);
  static void outputConfigPresetsStyles(std::ofstream&);
  static void outputConfigPresetsBlades(std::ofstream&);
  static void genWS281X(std::ofstream&, const BladesPage::BladeConfig&);
  static void outputConfigButtons(std::ofstream&);

  static void readConfigTop(std::ifstream&);
  static void readConfigProp(std::ifstream&);
  static void readConfigPresets(std::ifstream&);
  static void readConfigStyles(std::ifstream&);
  static void replaceStyles(const std::string&, const std::string&);
  static void readPresetArray(std::ifstream&);
  static void readBladeArray(std::ifstream&);
};
