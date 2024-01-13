// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/pages/bladespage.h"
#include "editor/editorwindow.h"

#include <string>
#include <fstream>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>

class Configuration {
public:
  static bool outputConfig(EditorWindow* editorWindow);
  static bool outputConfig(const std::string&, EditorWindow* editorWindow);
  static bool exportConfig(EditorWindow* editorWindow);
  static bool readConfig(const std::string&, EditorWindow* editorWindow);
  static bool importConfig(EditorWindow* editorWindow);

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

  static bool runPreChecks(EditorWindow*);

  static void outputConfigTop(std::ofstream&, EditorWindow*);
  static void outputConfigTopGeneral(std::ofstream&, EditorWindow*);
  static void outputConfigTopBladeAwareness(std::ofstream&, EditorWindow* configOutput);
  static void outputConfigTopPropSpecific(std::ofstream&, EditorWindow*);
  static void outputConfigTopSA22C(std::ofstream&, EditorWindow*);
  static void outputConfigTopFett263(std::ofstream&, EditorWindow*);
  static void outputConfigTopBC(std::ofstream&, EditorWindow*);
  static void outputConfigTopCaiwyn(std::ofstream&, EditorWindow*);
  static void outputConfigProp(std::ofstream&, EditorWindow*);
  static void outputConfigPresets(std::ofstream&, EditorWindow*);
  static void outputConfigPresetsStyles(std::ofstream&, EditorWindow*);
  static void outputConfigPresetsBlades(std::ofstream&, EditorWindow*);
  static void genWS281X(std::ofstream&, const BladesPage::BladeConfig&);
  static void genSubBlades(std::ofstream&, const BladesPage::BladeConfig&);
  static void outputConfigButtons(std::ofstream&, EditorWindow*);

  static void readConfigTop(std::ifstream&, EditorWindow*);
  static void readConfigProp(std::ifstream&, EditorWindow*);
  static void readConfigPresets(std::ifstream&, EditorWindow*);
  static void readConfigStyles(std::ifstream&, EditorWindow*);
  static void replaceStyles(const std::string&, const std::string&, EditorWindow*);
  static void readPresetArray(std::ifstream&, EditorWindow*);
  static void readBladeArray(std::ifstream&, EditorWindow*);
};
