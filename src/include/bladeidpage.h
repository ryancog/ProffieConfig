#pragma once

#include "misc.h"

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
  wxTextCtrl* IDPin{nullptr};
  Misc::numEntry* pullupResistance{nullptr};
  wxTextCtrl* pullupPin{nullptr};

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

  wxTextCtrl* arrayName{nullptr};
  Misc::numEntry* resistanceID{nullptr};

  wxTextCtrl* detectPin{nullptr};

private:
  wxStaticBoxSizer* createBladeArrays(wxWindow*);
  wxBoxSizer* createBladeArraysLeft(wxWindow*);
  wxBoxSizer* createBladeArraysRight(wxStaticBoxSizer*);

  wxStaticBoxSizer* createIDSetup(wxWindow*);
  wxStaticBoxSizer* createIDPowerSettings(wxWindow*);
  wxStaticBoxSizer* createContinuousScanSettings(wxWindow*);
  wxStaticBoxSizer* createBladeDetect(wxWindow*);
};
