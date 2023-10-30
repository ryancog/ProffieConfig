#pragma once
#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>

#include "misc.h"

class PropPage : public wxStaticBoxSizer {
public:
  PropPage(wxWindow*);
  static PropPage* instance;

  void update();

  struct {
    wxComboBox* prop{nullptr};
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
  } settings;

private:
  class RStaticBox : public wxStaticBoxSizer {
  public:
    RStaticBox(int, wxWindow*, const wxString&);
  };

  std::vector<RStaticBox*> boxes;

  RStaticBox* gestures(wxStaticBoxSizer*);
  RStaticBox* stabOn(wxStaticBoxSizer*);
  RStaticBox* swingOn(wxStaticBoxSizer*);
  RStaticBox* thrustOn(wxStaticBoxSizer*);
  RStaticBox* twistOn(wxStaticBoxSizer*);
  RStaticBox* twistOff(wxStaticBoxSizer*);

  RStaticBox* controls(wxStaticBoxSizer*);
  RStaticBox* generalControls(wxStaticBoxSizer*);
  RStaticBox* editMode(wxStaticBoxSizer*);
  RStaticBox* interfaceOptions(wxStaticBoxSizer*);

  RStaticBox* features(wxStaticBoxSizer*);
  RStaticBox* forcePush(wxStaticBoxSizer*);
  RStaticBox* quotePlayer(wxStaticBoxSizer*);
  RStaticBox* generalFeatures(wxStaticBoxSizer*);

  RStaticBox* battleMode(wxStaticBoxSizer*);
  RStaticBox* activation(wxStaticBoxSizer*);
  RStaticBox* lockup(wxStaticBoxSizer*);
  RStaticBox* bmControls(wxStaticBoxSizer*);
};
