// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once
#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>

#include "misc.h"

class PropPage : public wxScrolledWindow {
public:
  PropPage(wxWindow*);
  static PropPage* instance;

  void update();
  wxStaticBoxSizer* sizer{nullptr};

  wxComboBox* prop{nullptr};
  wxButton* buttonInfo{nullptr};

  wxCheckBox* noLockupHold{nullptr};

  wxCheckBox* disableGestureNoBlade{nullptr};
  wxCheckBox* gestureBattle{nullptr};
  // Stab On
  wxCheckBox* stabOn{nullptr};
  wxRadioButton* stabOnFast{nullptr};
  wxRadioButton* stabOnPreon{nullptr};
  wxCheckBox* stabOnNoBattle{nullptr};
  // Swing On
  wxCheckBox* swingOn{nullptr};
  Misc::numEntry* swingOnSpeed;
  wxRadioButton* swingOnFast{nullptr};
  wxRadioButton* swingOnPreon{nullptr};
  wxCheckBox* swingOnNoBattle{nullptr};
  // Twist On
  wxCheckBox* twistOn{nullptr};
  wxRadioButton* twistOnFast{nullptr};
  wxRadioButton* twistOnPreon{nullptr};
  wxCheckBox* twistOnNoBattle{nullptr};
  // Thrust On
  wxCheckBox* thrustOn{nullptr};
  wxRadioButton* thrustOnFast{nullptr};
  wxRadioButton* thrustOnPreon{nullptr};
  wxCheckBox* thrustOnNoBattle{nullptr};
  // Twist Off
  wxCheckBox* twistOff{nullptr};
  wxRadioButton* twistOffFast{nullptr};
  wxRadioButton* twistOffPostoff{nullptr};

  // Battle Mode
  wxCheckBox* gestureEnBattle{nullptr};
  Misc::numEntry* lockupDelay;
  wxRadioButton* battleModeToggle{nullptr};
  wxRadioButton* battleModeAlways{nullptr};
  wxRadioButton* battleModeOnStart{nullptr};
  wxCheckBox* battleModeDisablePWR{nullptr};
  Misc::numEntryDouble* battleModeClash;

  // Force Push
  wxCheckBox* forcePush{nullptr};
  wxCheckBox* forcePushBM{nullptr};
  Misc::numEntry* forcePushLength;

  // Edit Mode/Settings
  wxCheckBox* editEnable{nullptr};
  wxRadioButton* editMode{nullptr};
  wxRadioButton* editSettings{nullptr};

  // Quote Player
  wxCheckBox* enableQuotePlayer{nullptr};
  wxCheckBox* randomizeQuotePlayer{nullptr};
  wxRadioButton* forcePlayerDefault{nullptr};
  wxRadioButton* quotePlayerDefault{nullptr};

  wxCheckBox* pwrClash{nullptr};
  wxCheckBox* pwrLockup{nullptr};
  wxCheckBox* pwrHoldOff{nullptr};
  wxCheckBox* auxHoldLockup{nullptr};
  wxCheckBox* meltGestureAlways{nullptr};
  wxCheckBox* volumeCircular{nullptr};
  wxCheckBox* brightnessCircular{nullptr};
  wxCheckBox* pwrWakeGesture{nullptr};
  wxRadioButton* noExtraEffects{nullptr};
  wxRadioButton* specialAbilities{nullptr};
  wxRadioButton* multiPhase{nullptr};
  wxCheckBox* spinMode{nullptr};
  wxCheckBox* saveGesture{nullptr};
  wxCheckBox* saveChoreo{nullptr};
  wxCheckBox* dualModeSound{nullptr};
  wxCheckBox* clashStrengthSound{nullptr};
  Misc::numEntry* clashStrengthSoundMaxClash;
  wxCheckBox* quickPresetSelect{nullptr};
  wxCheckBox* spokenColors{nullptr};
  wxRadioButton* spokenBatteryNone{nullptr};
  wxRadioButton* spokenBatteryVolts{nullptr};
  wxRadioButton* spokenBatteryPercent{nullptr};

  wxCheckBox* beepErrors{nullptr};
  wxCheckBox* trackPlayerPrompts{nullptr};
  wxCheckBox* fontChangeOTF{nullptr};
  wxCheckBox* styleChangeOTF{nullptr};
  wxCheckBox* presetCopyOTF{nullptr};
  wxCheckBox* battleModeNoToggle{nullptr};
  wxCheckBox* multiBlast{nullptr};
  wxCheckBox* multiBlastDisableToggle{nullptr};
  wxCheckBox* multiBlastSwing{nullptr};

private:
  enum {
    ID_Select,
    ID_Option,
    ID_Buttons
  };

  class PropPageBox : public wxStaticBoxSizer {
  public:
    PropPageBox(int32_t, wxWindow*, const wxString&);
  };

  std::vector<PropPageBox*> boxes;

  void bindEvents();
  void createToolTips();

  PropPageBox* createGestures(wxStaticBoxSizer*);
  PropPageBox* createStabOn(wxStaticBoxSizer*);
  PropPageBox* createSwingOn(wxStaticBoxSizer*);
  PropPageBox* createThrustOn(wxStaticBoxSizer*);
  PropPageBox* createTwistOn(wxStaticBoxSizer*);
  PropPageBox* createTwistOff(wxStaticBoxSizer*);

  PropPageBox* createControls(wxStaticBoxSizer*);
  PropPageBox* createGeneralControls(wxStaticBoxSizer*);
  PropPageBox* createEditMode(wxStaticBoxSizer*);
  PropPageBox* createInterfaceOptions(wxStaticBoxSizer*);

  PropPageBox* createFeatures(wxStaticBoxSizer*);
  PropPageBox* createForcePush(wxStaticBoxSizer*);
  PropPageBox* createQuotePlayer(wxStaticBoxSizer*);
  PropPageBox* createGeneralFeatures(wxStaticBoxSizer*);

  PropPageBox* createBattleMode(wxStaticBoxSizer*);
  PropPageBox* createActivation(wxStaticBoxSizer*);
  PropPageBox* createLockup(wxStaticBoxSizer*);
  PropPageBox* createBMControls(wxStaticBoxSizer*);
};
