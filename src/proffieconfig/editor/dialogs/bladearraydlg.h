#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../pages/presetspage.h"
#include "../pages/bladespage.h"
#include "ui/controls.h"

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/listbox.h>

enum {
    BLADE_ID_MODE_SNAPSHOT = 0,
    BLADE_ID_MODE_EXTERNAL,
    BLADE_ID_MODE_BRIDGED,
};

class BladeArrayDlg : public wxDialog {
public:
  BladeArrayDlg(EditorWindow*);

  void update();

  wxCheckBox* enableID{nullptr};
  wxCheckBox* enableDetect{nullptr};

  PCUI::Choice* mode{nullptr};
  PCUI::Text* IDPin{nullptr};
  PCUI::Numeric* pullupResistance{nullptr};
  PCUI::Text* pullupPin{nullptr};

  wxCheckBox* enablePowerForID{nullptr};
  wxCheckBox* powerPin1{nullptr};
  wxCheckBox* powerPin2{nullptr};
  wxCheckBox* powerPin3{nullptr};
  wxCheckBox* powerPin4{nullptr};
  wxCheckBox* powerPin5{nullptr};
  wxCheckBox* powerPin6{nullptr};

  wxCheckBox* continuousScans{nullptr};
  PCUI::Numeric* numIDTimes{nullptr};
  PCUI::Numeric* scanIDMillis{nullptr};

  wxListBox* arrayList{nullptr};
  wxButton* addID{nullptr};
  wxButton* removeID{nullptr};

  PCUI::Text* arrayName{nullptr};
  PCUI::Numeric* resistanceID{nullptr};

  PCUI::Text* detectPin{nullptr};

  struct BladeArray {
    string name;
    int32_t value{0};

    std::vector<PresetsPage::PresetConfig> presets;
    std::vector<BladesPage::BladeConfig> blades;
  };
  std::vector<BladeArray> bladeArrays{BladeArray{"blade_in", 0}};

  enum {
    ID_NameEntry,
    ID_BladeIDEnable,
    ID_BladeDetectEnable,
    ID_BladeIDMode,
    ID_BladeArray,
    ID_BladeIDPower,
    ID_ContinuousScan,
    ID_RemoveArray,
    ID_AddArray
  };

private:
  EditorWindow* mParent{nullptr};
  wxBoxSizer* mSizer{nullptr};
  int32 mLastArraySelection{-1};

  void bindEvents();
  void createToolTips() const;

  void stripAndSaveName();

  wxStaticBoxSizer* createBladeArrays(wxWindow*);
  wxBoxSizer* createBladeArraysLeft(wxWindow*);
  wxBoxSizer* createBladeArraysRight(wxStaticBoxSizer*);

  wxStaticBoxSizer* createIDSetup(wxWindow*);
  wxStaticBoxSizer* createIDPowerSettings(wxWindow*);
  wxStaticBoxSizer* createContinuousScanSettings(wxWindow*);
  wxStaticBoxSizer* createBladeDetect(wxWindow*);
};
