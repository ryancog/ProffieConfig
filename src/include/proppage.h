#pragma once
#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>

#include "misc.h"

class PropPage : public wxStaticBoxSizer {
public:
  PropPage(wxWindow*);
  static void updatePropOptions();

  struct {
    wxComboBox *prop{nullptr};
    wxCheckBox *noLockupHold{nullptr};

    wxCheckBox *disableGuestureNoBlade{nullptr};
    wxCheckBox *guestureBattle{nullptr};
    // Stab On
    wxCheckBox *stabOn{nullptr};
    wxRadioButton *stabOnFast{nullptr};
    wxRadioButton *stabOnPreon{nullptr};
    wxCheckBox *stabOnNoBattle{nullptr};
    // Swing On
    wxCheckBox *swingOn{nullptr};
    Misc::numEntry swingOnSpeed;
    wxRadioButton *swingOnFast{nullptr};
    wxRadioButton *swingOnPreon{nullptr};
    wxCheckBox *swingOnNoBattle{nullptr};
    // Twist On
    wxCheckBox *twistOn{nullptr};
    wxRadioButton *twistOnFast{nullptr};
    wxRadioButton *twistOnPreon{nullptr};
    wxCheckBox *twistOnNoBattle{nullptr};
    // Thrust On
    wxCheckBox *thrustOn{nullptr};
    wxRadioButton *thrustOnFast{nullptr};
    wxRadioButton *thrustOnPreon{nullptr};
    wxCheckBox *thrustOnNoBattle{nullptr};
    // Twist Off
    wxCheckBox *twistOff{nullptr};
    wxRadioButton *twistOffFast{nullptr};
    wxRadioButton *twistOffPostoff{nullptr};

    // Battle Mode
    wxCheckBox *gestureEnBattle{nullptr};
    Misc::numEntry lockupDelay;
    wxRadioButton *battleModeToggle{nullptr};
    wxRadioButton *battleModeAlways{nullptr};
    wxRadioButton *battleModeOnStart{nullptr};
    wxCheckBox *battleModeDisablePWR{nullptr};
    Misc::numEntryDouble battleModeClash;

    // Force Push
    wxCheckBox *forcePush{nullptr};
    wxCheckBox *forcePushAlways{nullptr};
    Misc::numEntry forcePushLength;

    // Edit Mode/Settings
    wxRadioButton *editMode{nullptr};
    wxRadioButton *editSettings{nullptr};

    // Quote Player
    wxCheckBox *enableQuotePlayer{nullptr};
    wxCheckBox *randomizeQuotePlayer{nullptr};
    wxRadioButton *forcePlayerDefault{nullptr};
    wxRadioButton *quotePlayerDefault{nullptr};

    wxCheckBox *pwrClash{nullptr};
    wxCheckBox *pwrLockup{nullptr};
    wxCheckBox *pwrHoldOff{nullptr};
    wxCheckBox *auxHoldLockup{nullptr};
    wxCheckBox *meltGuestureAlways{nullptr};
    wxCheckBox *volumeCircular{nullptr};
    wxCheckBox *brightnessCircular{nullptr};
    wxCheckBox *pwrWakeGuesture{nullptr};
    wxRadioButton *specialAbilities{nullptr};
    wxRadioButton *multiPhase{nullptr};
    wxCheckBox *spinMode{nullptr};
    wxCheckBox *saveGuestureDisable{nullptr};
    wxCheckBox *saveChoreo{nullptr};
    wxCheckBox *dualModeSound{nullptr};
    wxCheckBox *clashStrengthSound{nullptr};
    Misc::numEntry maxClash;
    wxCheckBox *quickPresetSelect{nullptr};
    wxCheckBox *spokenColors{nullptr};
    wxRadioButton *spokenBatteryNone{nullptr};
    wxRadioButton *spokenBatteryVolts{nullptr};
    wxRadioButton *spokenBatteryPercent{nullptr};

    wxCheckBox *beepErrors{nullptr};
    wxCheckBox *trackPlayerPrompts{nullptr};
    wxCheckBox *fontChangeOTF{nullptr};
    wxCheckBox *styleChangeOTF{nullptr};
    wxCheckBox *presetCopyOTF{nullptr};
    wxCheckBox *battleToggle{nullptr};
    wxCheckBox *multiBlast{nullptr};
    wxCheckBox *multiBlastSwing{nullptr};
    wxCheckBox *multiBlastToggle{nullptr};
  } static settings;

private:
  void createPropSettings();

  wxStaticBoxSizer* guestures(wxStaticBoxSizer*);
  wxStaticBoxSizer* stabOn(wxStaticBoxSizer*);
  wxStaticBoxSizer* swingOn(wxStaticBoxSizer*);
  wxStaticBoxSizer* thrustOn(wxStaticBoxSizer*);
  wxStaticBoxSizer* twistOn(wxStaticBoxSizer*);
  wxStaticBoxSizer* twistOff(wxStaticBoxSizer*);

  wxStaticBoxSizer* controls(wxStaticBoxSizer*);

  wxStaticBoxSizer* features(wxStaticBoxSizer*);
  wxStaticBoxSizer* forcePush(wxStaticBoxSizer*);
  wxStaticBoxSizer* battleMode(wxStaticBoxSizer*);

};
