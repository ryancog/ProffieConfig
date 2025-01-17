// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

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
  Configuration(Configuration &&) = delete;
  static bool outputConfig(const EditorWindow *editorWindow);
  static bool outputConfig(const std::string&, const EditorWindow *editorWindow);
  static bool exportConfig(EditorWindow *editorWindow);
  static bool readConfig(const std::string&, EditorWindow *editorWindow);
  static bool importConfig(EditorWindow *editorWindow);

  typedef std::pair<const std::string, const std::string> MapPair;
  typedef std::vector<MapPair> VMap;
  static const MapPair& findInVMap(const VMap&, const std::string& search);

  static inline const VMap Orientation = {
    { "FETs Towards Blade", "ORIENTATION_FETS_TOWARDS_BLADE" },
    { "USB Towards Blade", "ORIENTATION_USB_TOWARDS_BLADE" },
    { "USB CCW From Blade", "ORIENTATION_USB_CCW_FROM_BLADE" },
    { "USB CW From Blade", "ORIENTATION_USB_CW_FROM_BLADE" },
    { "Top Towards Blade", "ORIENTATION_TOP_TOWARDS_BLADE" },
    { "Bottom Towards Blade", "ORIENTATION_BOTTOM_TOWARDS_BLADE" },
    };
  static inline const VMap Proffieboard = {
    { "ProffieBoard V1", "#include \"proffieboard_v1_config.h\"" },
    { "ProffieBoard V2", "#include \"proffieboard_v2_config.h\"" },
    { "ProffieBoard V3", "#include \"proffieboard_v3_config.h\"" },
    };

private:
  Configuration();
  Configuration(const Configuration&) = delete;

  static bool runPreChecks(const EditorWindow *);

  static void outputConfigTop(std::ofstream&, const EditorWindow *);
  static void outputConfigTopGeneral(std::ofstream&, const EditorWindow *);
  static void outputConfigTopCustom(std::ofstream&, const EditorWindow *);
  static void outputConfigTopBladeAwareness(std::ofstream&, const EditorWindow *);
  static void outputConfigTopPropSpecific(std::ofstream&, const EditorWindow *);
  static void outputConfigTopSA22C(std::ofstream&, const EditorWindow *);
  static void outputConfigTopFett263(std::ofstream&, const EditorWindow *);
  static void outputConfigTopBC(std::ofstream&, const EditorWindow *);
  static void outputConfigTopCaiwyn(std::ofstream&, const EditorWindow *);
  static void outputConfigProp(std::ofstream&, const EditorWindow *);
  static void outputConfigPresets(std::ofstream&, const EditorWindow *);
  static void outputConfigPresetsStyles(std::ofstream&, const EditorWindow *);
  static void outputConfigPresetsBlades(std::ofstream&, const EditorWindow *);
  static void genWS281X(std::ofstream&, const BladesPage::BladeConfig&);
  static void genSubBlades(std::ofstream&, const BladesPage::BladeConfig&);
  static void outputConfigButtons(std::ofstream&, const EditorWindow *);

  static void readConfigTop(std::ifstream&, EditorWindow*);
  static void readConfigProp(std::ifstream&, EditorWindow*);
  static void readConfigPresets(std::ifstream&, EditorWindow*);
  static void readConfigStyles(std::ifstream&, EditorWindow*);
  static void replaceStyles(const std::string&, const std::string&, EditorWindow*);
  static void readPresetArray(std::ifstream&, EditorWindow*);
  static void readBladeArray(std::ifstream&, EditorWindow*);
  static void setCustomDefines(EditorWindow* editor);
};
