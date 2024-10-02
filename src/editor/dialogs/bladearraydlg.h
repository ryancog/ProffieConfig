// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/pages/presetspage.h"
#include "editor/pages/bladespage.h"
#include "ui/pctextctrl.h"

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/listbox.h>

constexpr auto BLADE_ID_MODE_SNAPSHOT{"Snapshot"};
constexpr auto BLADE_ID_MODE_EXTERNAL{"External Pullup"};
constexpr auto BLADE_ID_MODE_BRIDGED {"Bridged Pullup"};

class BladeArrayDlg : public wxDialog {
public:
  BladeArrayDlg(EditorWindow*);

  void update();

  wxCheckBox* enableID{nullptr};
  wxCheckBox* enableDetect{nullptr};

  pcChoice* mode{nullptr};
  pcTextCtrl* IDPin{nullptr};
  pcSpinCtrl* pullupResistance{nullptr};
  pcTextCtrl* pullupPin{nullptr};

  wxCheckBox* enablePowerForID{nullptr};
  wxCheckBox* powerPin1{nullptr};
  wxCheckBox* powerPin2{nullptr};
  wxCheckBox* powerPin3{nullptr};
  wxCheckBox* powerPin4{nullptr};
  wxCheckBox* powerPin5{nullptr};
  wxCheckBox* powerPin6{nullptr};

  wxCheckBox* continuousScans{nullptr};
  pcSpinCtrl* numIDTimes{nullptr};
  pcSpinCtrl* scanIDMillis{nullptr};

  wxListBox* arrayList{nullptr};
  wxButton* addID{nullptr};
  wxButton* removeID{nullptr};

  pcTextCtrl* arrayName{nullptr};
  pcSpinCtrl* resistanceID{nullptr};

  pcTextCtrl* detectPin{nullptr};

  struct BladeArray {
    wxString name{""};
    int32_t value{0};

    std::vector<PresetsPage::PresetConfig> presets{};
    std::vector<BladesPage::BladeConfig> blades{};
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
  EditorWindow* parent{nullptr};
  wxBoxSizer* sizer{nullptr};
  int32_t lastArraySelection{-1};

  void bindEvents();
  void createToolTips();

  void stripAndSaveName();

  wxStaticBoxSizer* createBladeArrays(wxWindow*);
  wxBoxSizer* createBladeArraysLeft(wxWindow*);
  wxBoxSizer* createBladeArraysRight(wxStaticBoxSizer*);

  wxStaticBoxSizer* createIDSetup(wxWindow*);
  wxStaticBoxSizer* createIDPowerSettings(wxWindow*);
  wxStaticBoxSizer* createContinuousScanSettings(wxWindow*);
  wxStaticBoxSizer* createBladeDetect(wxWindow*);
};
