#pragma once

#include "misc.h"
#include "presetspage.h"
#include "bladespage.h"

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/listbox.h>

#define BLADE_ID_MODE_SNAPSHOT "Snapshot"
#define BLADE_ID_MODE_EXTERNAL "External Pullup"
#define BLADE_ID_MODE_BRIDGED  "Bridged Pullup"

class BladeIDPage : public wxStaticBoxSizer {
public:
  BladeIDPage(wxWindow*);
  static BladeIDPage* instance;

  void update();

  wxCheckBox* enableID{nullptr};
  wxCheckBox* enableDetect{nullptr};

  wxComboBox* mode{nullptr};
  Misc::textEntry* IDPin{nullptr};
  Misc::numEntry* pullupResistance{nullptr};
  Misc::textEntry* pullupPin{nullptr};

  wxCheckBox* enablePowerForID{nullptr};
  wxCheckBox* powerPin1{nullptr};
  wxCheckBox* powerPin2{nullptr};
  wxCheckBox* powerPin3{nullptr};
  wxCheckBox* powerPin4{nullptr};
  wxCheckBox* powerPin5{nullptr};
  wxCheckBox* powerPin6{nullptr};

  wxCheckBox* continuousScans{nullptr};
  Misc::numEntry* numIDTimes{nullptr};
  Misc::numEntry* scanIDMillis{nullptr};

  wxListBox* arrayList{nullptr};
  wxButton* addID{nullptr};
  wxButton* removeID{nullptr};

  Misc::textEntry* arrayName{nullptr};
  Misc::numEntry* resistanceID{nullptr};

  Misc::textEntry* detectPin{nullptr};

  struct BladeArray {
    wxString name{"blade_in"};
    int32_t value{0};

    std::vector<PresetsPage::PresetConfig> presets{};
    std::vector<BladesPage::BladeConfig> blades{};
  };
  std::vector<BladeArray> bladeArrays{BladeArray{}};

private:
  enum {
    ID_BladeIDEnable,
    ID_BladeDetectEnable,
    ID_BladeIDMode,
    ID_BladeArray,
    ID_BladeIDPower,
    ID_ContinuousScan,
    ID_RemoveArray,
    ID_AddArray
  };
  int32_t lastArraySelection{-1};

  void bindEvents();

  wxStaticBoxSizer* createBladeArrays(wxWindow*);
  wxBoxSizer* createBladeArraysLeft(wxWindow*);
  wxBoxSizer* createBladeArraysRight(wxStaticBoxSizer*);

  wxStaticBoxSizer* createIDSetup(wxWindow*);
  wxStaticBoxSizer* createIDPowerSettings(wxWindow*);
  wxStaticBoxSizer* createContinuousScanSettings(wxWindow*);
  wxStaticBoxSizer* createBladeDetect(wxWindow*);
};
