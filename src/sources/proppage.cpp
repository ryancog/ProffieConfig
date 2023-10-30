#include "proppage.h"
#include <wx/sizer.h>

#include "misc.h"
#include "defines.h"
#include "generalpage.h"

PropPage* PropPage::instance;
PropPage::PropPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "") {
  instance = this;
  settings.prop = new wxComboBox(this->GetStaticBox(), Misc::ID_PropSelect, PR_DEFAULT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({PR_DEFAULT, PR_SA22C, PR_FETT263, PR_SHTOK, PR_BC, PR_CAIWYN}), wxCB_READONLY);

  Add(settings.prop, MENUITEMFLAGS);
  Add(gestures(this), BOXITEMFLAGS);
  Add(controls(this), BOXITEMFLAGS);
  Add(features(this), BOXITEMFLAGS);
  Add(battleMode(this), BOXITEMFLAGS);
}

void PropPage::update() {
# define SA22C prop == PR_SA22C
# define FETT263 prop == PR_FETT263
# define BC prop == PR_BC
# define SHTOK prop == PR_SHTOK
# define CAIWYN prop == PR_CAIWYN
# define ALL SA22C || FETT263 || BC || SHTOK || CAIWYN

  std::string prop = settings.prop->GetStringSelection().ToStdString();

  settings.disableGestureNoBlade->Show(BC);
  settings.noLockupHold->Show(SA22C);
  // Stab On
  settings.stabOn->Show(SA22C || FETT263 || BC);
  settings.stabOnFast->Show(FETT263);
  settings.stabOnPreon->Show(FETT263);
  settings.stabOnNoBattle->Show(FETT263);
  // Swing On
  settings.swingOn->Show(SA22C || FETT263 || BC);
  settings.swingOnSpeed->box->Show(SA22C || FETT263 || BC);
  settings.swingOnSpeed->num->Enable(settings.swingOn->GetValue());
  settings.swingOnFast->Show(FETT263);
  settings.swingOnPreon->Show(FETT263);
  settings.swingOnNoBattle->Show(FETT263);
  // Twist On
  settings.twistOn->Show(SA22C || FETT263 || BC);
  settings.twistOnFast->Show(FETT263);
  settings.twistOnPreon->Show(FETT263);
  settings.twistOnNoBattle->Show(FETT263);
  // Thrust On
  settings.thrustOn->Show(SA22C || FETT263 || BC);
  settings.thrustOnFast->Show(FETT263);
  settings.thrustOnPreon->Show(FETT263);
  settings.thrustOnNoBattle->Show(FETT263);
  // Twist Off
  settings.twistOff->Show(SA22C || FETT263 || BC);
  settings.twistOffFast->Show(FETT263);
  settings.twistOffPostoff->Show(FETT263);

  // Battle Mode
  settings.gestureEnBattle->Show(SA22C || BC);
  settings.lockupDelay->box->Show(SA22C || FETT263 || BC);
  settings.battleModeToggle->Show(FETT263);
  settings.battleModeAlways->Show(FETT263);
  settings.battleModeOnStart->Show(FETT263);
  settings.battleModeDisablePWR->Show(FETT263);
  settings.battleModeClash->box->Show(FETT263);

  // Force Push
  settings.forcePush->Show(SA22C || FETT263 || BC);
  settings.forcePushBM->Show(FETT263);
  settings.forcePushLength->box->Show(SA22C || FETT263 || BC);
  if (settings.forcePush->GetValue()) {
    settings.forcePushBM->SetValue(true);
    settings.forcePushBM->Disable();
  } else settings.forcePushBM->Enable();
  if (settings.forcePushBM->GetValue()) settings.forcePushLength->num->Enable();
  else settings.forcePushLength->num->Disable();


  // Edit Mode/Settings

  settings.editEnable->Show(FETT263);
  settings.editMode->Show(FETT263);
  settings.editSettings->Show(FETT263);
  if (settings.editEnable->GetValue()) {
    settings.editMode->Enable();
    settings.editSettings->Enable();
  } else {
    settings.editMode->Disable();
    settings.editSettings->Disable();
  }

  // Quote Player
  settings.enableQuotePlayer->Show(FETT263);
  settings.randomizeQuotePlayer->Show(FETT263);
  settings.forcePlayerDefault->Show(FETT263);
  settings.quotePlayerDefault->Show(FETT263);
  if (settings.enableQuotePlayer->GetValue()) {
    settings.randomizeQuotePlayer->Enable();
    settings.forcePlayerDefault->Enable();
    settings.quotePlayerDefault->Enable();
  } else {
    settings.randomizeQuotePlayer->Disable();
    settings.forcePlayerDefault->Disable();
    settings.forcePlayerDefault->SetValue(true);
    settings.quotePlayerDefault->Disable();
  }

  settings.pwrClash->Show(CAIWYN);
  settings.pwrLockup->Show(CAIWYN);
  settings.pwrHoldOff->Show(FETT263);
  if (GeneralPage::instance->settings.buttons->num->GetValue() == 2) settings.pwrHoldOff->Enable();
  else settings.pwrHoldOff->Disable();
  settings.auxHoldLockup->Show(FETT263);
  if (GeneralPage::instance->settings.buttons->num->GetValue() == 2) settings.auxHoldLockup->Enable();
  else settings.auxHoldLockup->Disable();

  settings.meltGestureAlways->Show(FETT263);
  settings.volumeCircular->Show(FETT263);
  settings.brightnessCircular->Show(FETT263);
  settings.pwrWakeGesture->Show(FETT263);

  settings.noExtraEffects->Show(FETT263);
  settings.specialAbilities->Show(FETT263);
  if (settings.saveChoreo->GetValue()) {
    if (!settings.multiPhase->GetValue()) settings.noExtraEffects->SetValue(true);
    settings.specialAbilities->Disable();
  } else settings.specialAbilities->Enable();
  settings.multiPhase->Show(FETT263);

  settings.spinMode->Show(FETT263);
  if (settings.auxHoldLockup->GetValue() || settings.saveChoreo->GetValue()) settings.spinMode->Disable();
  else settings.spinMode->Enable();

  settings.saveGesture->Show(FETT263);
  settings.saveChoreo->Show(FETT263);
  settings.dualModeSound->Show(FETT263);
  settings.clashStrengthSound->Show(FETT263);
  settings.clashStrengthSoundMaxClash->box->Show(FETT263);
  if (settings.clashStrengthSound->GetValue()) settings.clashStrengthSoundMaxClash->num->Enable();
  else settings.clashStrengthSoundMaxClash->num->Disable();
  settings.quickPresetSelect->Show(FETT263);
  settings.spokenColors->Show(FETT263);
  settings.spokenBatteryNone->Show(FETT263);
  settings.spokenBatteryVolts->Show(FETT263);
  settings.spokenBatteryPercent->Show(FETT263);

  // Disables
  settings.beepErrors->Show(FETT263);
  settings.trackPlayerPrompts->Show(FETT263);
  settings.fontChangeOTF->Show(FETT263);
  settings.styleChangeOTF->Show(FETT263);
  settings.presetCopyOTF->Show(FETT263);
  settings.battleModeNoToggle->Show(FETT263);
  settings.multiBlast->Show(FETT263);
  settings.multiBlastDisableToggle->Show(FETT263);
  if (settings.multiBlast->GetValue()) settings.multiBlastDisableToggle->Enable();
  else settings.multiBlastDisableToggle->Disable();
  settings.multiBlastSwing->Show(BC);

  for(const RStaticBox* box : boxes) {
    bool shouldShow = false;
    for (const wxWindow* item : box->GetStaticBox()->GetChildren()) {
      shouldShow = shouldShow || item->IsShown();
    }
    box->GetStaticBox()->Show(shouldShow);
  }

# undef SA22C
# undef CAIWYN
# undef FETT263
# undef BC
# undef SHTOK
# undef ALL
}

PropPage::RStaticBox* PropPage::gestures(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* gestures = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Gesture Control");
  gestures->Add(stabOn(gestures), BOXITEMFLAGS);
  gestures->Add(swingOn(gestures), BOXITEMFLAGS);
  gestures->Add(thrustOn(gestures), BOXITEMFLAGS);
  gestures->Add(twistOn(gestures), BOXITEMFLAGS);
  gestures->Add(twistOff(gestures), BOXITEMFLAGS);

  return gestures;
}
PropPage::RStaticBox* PropPage::stabOn(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox *stabOn = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Stab On");
  settings.stabOn = new wxCheckBox(stabOn->GetStaticBox(), Misc::ID_PropOption, "Stab To Turn On");
  settings.stabOnFast = new wxRadioButton(stabOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.stabOnPreon = new wxRadioButton(stabOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.stabOnNoBattle = new wxCheckBox(stabOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  stabOn->Add(settings.stabOn, FIRSTITEMFLAGS);
  stabOn->Add(settings.stabOnFast, MENUITEMFLAGS);
  stabOn->Add(settings.stabOnPreon, MENUITEMFLAGS);
  stabOn->Add(settings.stabOnNoBattle, MENUITEMFLAGS);

  return stabOn;
}
PropPage::RStaticBox* PropPage::swingOn(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* swingOn = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Swing On");
  settings.swingOn = new wxCheckBox(swingOn->GetStaticBox(), Misc::ID_PropOption, "Swing To Turn On");
  settings.swingOnSpeed = Misc::createNumEntry(swingOn, "Swing on Speed", Misc::ID_PropOption, 50, 1000, 250);
  settings.swingOnFast = new wxRadioButton(swingOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.swingOnPreon = new wxRadioButton(swingOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.swingOnNoBattle = new wxCheckBox(swingOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  swingOn->Add(settings.swingOn, FIRSTITEMFLAGS);
  swingOn->Add(settings.swingOnFast, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnPreon, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnNoBattle, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnSpeed->box, MENUITEMFLAGS);

  return swingOn;
}
PropPage::RStaticBox* PropPage::thrustOn(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox *thrustOn = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Thrust On");
  settings.thrustOn = new wxCheckBox(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Thrust To Turn On");
  settings.thrustOnFast = new wxRadioButton(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.thrustOnPreon = new wxRadioButton(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.thrustOnNoBattle = new wxCheckBox(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  thrustOn->Add(settings.thrustOn, FIRSTITEMFLAGS);
  thrustOn->Add(settings.thrustOnFast, MENUITEMFLAGS);
  thrustOn->Add(settings.thrustOnPreon, MENUITEMFLAGS);
  thrustOn->Add(settings.thrustOnNoBattle, MENUITEMFLAGS);

  return thrustOn;
}
PropPage::RStaticBox* PropPage::twistOn(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox *twistOn = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Twist On");
  settings.twistOn = new wxCheckBox(twistOn->GetStaticBox(), Misc::ID_PropOption, "Twist To Turn On");
  settings.twistOnFast = new wxRadioButton(twistOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.twistOnPreon = new wxRadioButton(twistOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.twistOnNoBattle = new wxCheckBox(twistOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  twistOn->Add(settings.twistOn, FIRSTITEMFLAGS);
  twistOn->Add(settings.twistOnFast, MENUITEMFLAGS);
  twistOn->Add(settings.twistOnPreon, MENUITEMFLAGS);
  twistOn->Add(settings.twistOnNoBattle, MENUITEMFLAGS);

  return twistOn;
}
PropPage::RStaticBox* PropPage::twistOff(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox *twistOff = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Twist Off");
  settings.twistOff = new wxCheckBox(twistOff->GetStaticBox(), Misc::ID_PropOption, "Twist To Turn Off");
  settings.twistOffFast = new wxRadioButton(twistOff->GetStaticBox(), Misc::ID_PropOption, "Fast Retraction", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.twistOffPostoff = new wxRadioButton(twistOff->GetStaticBox(), Misc::ID_PropOption, "Enable Postoff");
  settings.twistOffPostoff->SetValue(true);
  twistOff->Add(settings.twistOff, FIRSTITEMFLAGS);
  twistOff->Add(settings.twistOffPostoff, MENUITEMFLAGS);
  twistOff->Add(settings.twistOffFast, MENUITEMFLAGS);

  return twistOff;
}

PropPage::RStaticBox* PropPage::controls(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* controls = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Controls");

  controls->Add(generalControls(controls), BOXITEMFLAGS);
  controls->Add(editMode(controls), BOXITEMFLAGS);
  controls->Add(interfaceOptions(controls), BOXITEMFLAGS);

  return controls;
}
PropPage::RStaticBox* PropPage::generalControls(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* generalControls = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "General");
  wxBoxSizer* generalControls1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalControls2 = new wxBoxSizer(wxVERTICAL);

  settings.pwrClash = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Press PWR to Clash");
  settings.pwrLockup = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Hold PWR to Lockup");
  settings.pwrHoldOff = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Hold PWR to Turn Off");
  settings.auxHoldLockup = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Hold AUX to Lockup");
  settings.disableGestureNoBlade = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Disable Gestures Without Blade");
  settings.noLockupHold = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Revert Lockup and Multi-Blast Trigger");
  generalControls1->Add(settings.noLockupHold, FIRSTITEMFLAGS);
  generalControls1->Add(settings.disableGestureNoBlade, MENUITEMFLAGS);
  generalControls1->Add(settings.pwrClash, MENUITEMFLAGS);
  generalControls1->Add(settings.pwrLockup, MENUITEMFLAGS);

  generalControls1->Add(settings.pwrHoldOff, FIRSTITEMFLAGS);
  generalControls1->Add(settings.auxHoldLockup, MENUITEMFLAGS);

  settings.meltGestureAlways = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Always use Melt Guesture");
  settings.volumeCircular = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Use Circular Volume Menu");
  settings.brightnessCircular = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "Use Circular Brightness Menu");
  settings.pwrWakeGesture = new wxCheckBox(generalControls->GetStaticBox(), Misc::ID_PropOption, "PWR After Timeout Enables Gestures");
  generalControls2->Add(settings.meltGestureAlways, FIRSTITEMFLAGS);
  generalControls2->Add(settings.volumeCircular, MENUITEMFLAGS);
  generalControls2->Add(settings.brightnessCircular, MENUITEMFLAGS);
  generalControls2->Add(settings.pwrWakeGesture, MENUITEMFLAGS);

  generalControls->Add(generalControls1);
  generalControls->Add(generalControls2);

  return generalControls;
}
PropPage::RStaticBox* PropPage::editMode(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* editMode = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Edit Mode");


  settings.editEnable = new wxCheckBox(editMode->GetStaticBox(), Misc::ID_PropOption, "Enable On-The-Fly Editing");
  settings.editMode = new wxRadioButton(editMode->GetStaticBox(), Misc::ID_PropOption, "Enable Edit Mode", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.editSettings = new wxRadioButton(editMode->GetStaticBox(), Misc::ID_PropOption, "Enable Edit Settings");
  editMode->Add(settings.editEnable, FIRSTITEMFLAGS);
  editMode->Add(settings.editMode, MENUITEMFLAGS);
  editMode->Add(settings.editSettings, MENUITEMFLAGS);

  return editMode;
}
PropPage::RStaticBox* PropPage::interfaceOptions(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* interface = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Interface");
  wxBoxSizer* interface1 = new wxBoxSizer(wxVERTICAL);

  settings.beepErrors = new wxCheckBox(interface->GetStaticBox(), Misc::ID_PropOption, "Beep Errors Instead of Spoken");
  settings.trackPlayerPrompts = new wxCheckBox(interface->GetStaticBox(), Misc::ID_PropOption, "Enable Track Player Prompts");
  settings.trackPlayerPrompts->SetValue(true);
  settings.spokenColors = new wxCheckBox(interface->GetStaticBox(), Misc::ID_PropOption, "Enable Spoken Colors");
  settings.spokenBatteryNone = new wxRadioButton(interface->GetStaticBox(), Misc::ID_PropOption, "Battery Speak None", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.spokenBatteryVolts = new wxRadioButton(interface->GetStaticBox(), Misc::ID_PropOption, "Battery Speak Voltage");
  settings.spokenBatteryPercent = new wxRadioButton(interface->GetStaticBox(), Misc::ID_PropOption, "Battery Speak Percentage");
  interface1->Add(settings.beepErrors, FIRSTITEMFLAGS);
  interface1->Add(settings.trackPlayerPrompts, MENUITEMFLAGS);
  interface1->Add(settings.spokenColors, MENUITEMFLAGS);
  interface1->Add(settings.spokenBatteryNone, MENUITEMFLAGS);
  interface1->Add(settings.spokenBatteryVolts, MENUITEMFLAGS);
  interface1->Add(settings.spokenBatteryPercent, MENUITEMFLAGS);

  interface->Add(interface1);

  return interface;
}

PropPage::RStaticBox* PropPage::features(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* features = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Features");

  features->Add(forcePush(features), BOXITEMFLAGS);
  features->Add(quotePlayer(features), BOXITEMFLAGS);
  features->Add(generalFeatures(features), BOXITEMFLAGS);

  return features;
}
PropPage::RStaticBox* PropPage::forcePush(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* forcePush = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Force Push");

  settings.forcePush = new wxCheckBox(forcePush->GetStaticBox(), Misc::ID_PropOption, "Enable Force Push");
  settings.forcePushLength = Misc::createNumEntry(forcePush, "Force Push Length", Misc::ID_PropOption, 0, 10, 5);
  forcePush->Add(settings.forcePush, FIRSTITEMFLAGS);
  forcePush->Add(settings.forcePushLength->box, MENUITEMFLAGS);

  return forcePush;
}
PropPage::RStaticBox* PropPage::quotePlayer(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* quotePlayer = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Quote Player");

  settings.enableQuotePlayer = new wxCheckBox(quotePlayer->GetStaticBox(), Misc::ID_PropOption, "Enable Quote Player");
  settings.enableQuotePlayer->SetValue(true);
  settings.randomizeQuotePlayer = new wxCheckBox(quotePlayer->GetStaticBox(), Misc::ID_PropOption, "Randomize Quotes");
  settings.forcePlayerDefault = new wxRadioButton(quotePlayer->GetStaticBox(), Misc::ID_PropOption, "Make Force FX Default");
  settings.forcePlayerDefault->SetValue(true);
  settings.quotePlayerDefault = new wxRadioButton(quotePlayer->GetStaticBox(), Misc::ID_PropOption, "Make Quotes Default");

  quotePlayer->Add(settings.enableQuotePlayer, FIRSTITEMFLAGS);
  quotePlayer->Add(settings.randomizeQuotePlayer, MENUITEMFLAGS);
  quotePlayer->Add(settings.forcePlayerDefault, MENUITEMFLAGS);
  quotePlayer->Add(settings.quotePlayerDefault, MENUITEMFLAGS);

  return quotePlayer;
}
PropPage::RStaticBox* PropPage::generalFeatures(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* generalFeatures = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Other");
  wxBoxSizer* generalFeatures1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalFeatures2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalFeatures3 = new wxBoxSizer(wxVERTICAL);

  settings.noExtraEffects = new wxRadioButton(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "No Extra Effects", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.specialAbilities = new wxRadioButton(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Special Abilities");
  settings.multiPhase = new wxRadioButton(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Multi-Phase Styles");

  settings.spinMode = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Toggle for Spin Mode");
  settings.saveChoreo = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Choreography");

  settings.fontChangeOTF = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "OTF Font Change");
  settings.fontChangeOTF->SetValue(true);
  settings.styleChangeOTF = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "OTF Style Change");
  settings.styleChangeOTF->SetValue(true);
  settings.presetCopyOTF = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "OTF Preset Copying");
  settings.presetCopyOTF->SetValue(true);

  settings.saveGesture = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Save \"Disable Gesture\"");
  settings.dualModeSound = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Ignition Sound Angle");
  settings.clashStrengthSound = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Clash Sound Strength");
  settings.clashStrengthSoundMaxClash = Misc::createNumEntry(generalFeatures, "CSS Max Clash", Misc::ID_PropOption, 8, 16, 10);
  settings.quickPresetSelect = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Preset Select on Boot");

  settings.multiBlast = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Multi-Blast");
  settings.multiBlast->SetValue(true);
  settings.multiBlastDisableToggle = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Multi-Blast Guesture Only");
  settings.multiBlastSwing = new wxCheckBox(generalFeatures->GetStaticBox(), Misc::ID_PropOption, "Multi-Blast On Swing");

  generalFeatures1->Add(settings.noExtraEffects, MENUITEMFLAGS);
  generalFeatures1->Add(settings.specialAbilities, MENUITEMFLAGS);
  generalFeatures1->Add(settings.multiPhase, MENUITEMFLAGS);
  generalFeatures1->Add(settings.spinMode, MENUITEMFLAGS);
  generalFeatures1->Add(settings.saveChoreo, MENUITEMFLAGS);

  generalFeatures2->Add(settings.saveGesture, MENUITEMFLAGS);
  generalFeatures2->Add(settings.dualModeSound, MENUITEMFLAGS);
  generalFeatures2->Add(settings.quickPresetSelect, MENUITEMFLAGS);
  generalFeatures2->Add(settings.multiBlast, MENUITEMFLAGS);
  generalFeatures2->Add(settings.multiBlastDisableToggle, MENUITEMFLAGS);
  generalFeatures2->Add(settings.multiBlastSwing, MENUITEMFLAGS);

  generalFeatures3->Add(settings.fontChangeOTF, MENUITEMFLAGS);
  generalFeatures3->Add(settings.styleChangeOTF, MENUITEMFLAGS);
  generalFeatures3->Add(settings.presetCopyOTF, MENUITEMFLAGS);
  generalFeatures3->Add(settings.clashStrengthSound, MENUITEMFLAGS);
  generalFeatures3->Add(settings.clashStrengthSoundMaxClash->box, MENUITEMFLAGS);

  generalFeatures->Add(generalFeatures1);
  generalFeatures->Add(generalFeatures2);
  generalFeatures->Add(generalFeatures3);

  return generalFeatures;
}

PropPage::RStaticBox* PropPage::battleMode(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* battleMode = new PropPage::RStaticBox(wxHORIZONTAL, parent->GetStaticBox(), "Battle Mode");

  battleMode->Add(activation(battleMode), BOXITEMFLAGS);
  battleMode->Add(lockup(battleMode), BOXITEMFLAGS);
  battleMode->Add(bmControls(battleMode), BOXITEMFLAGS);

  return battleMode;
}
PropPage::RStaticBox* PropPage::activation(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* activation = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Activation");

  settings.battleModeToggle = new wxRadioButton(activation->GetStaticBox(), Misc::ID_PropOption, "Battle Mode Toggle On", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.battleModeToggle->SetValue(true);
  settings.battleModeAlways = new wxRadioButton(activation->GetStaticBox(), Misc::ID_PropOption, "Battle Mode Always On");
  settings.battleModeOnStart = new wxRadioButton(activation->GetStaticBox(), Misc::ID_PropOption, "Battle Mode On Start");

  settings.battleModeNoToggle = new wxCheckBox(activation->GetStaticBox(), Misc::ID_PropOption, "Battle Mode Only On Gesture");

  settings.gestureEnBattle = new wxCheckBox(activation->GetStaticBox(), Misc::ID_PropOption, "Gesture Ignition Starts Battle Mode");

  activation->Add(settings.battleModeToggle, FIRSTITEMFLAGS);
  activation->Add(settings.battleModeOnStart, MENUITEMFLAGS);
  activation->Add(settings.battleModeAlways, MENUITEMFLAGS);

  activation->Add(settings.battleModeNoToggle, FIRSTITEMFLAGS);

  activation->Add(settings.gestureEnBattle, FIRSTITEMFLAGS);

  return activation;
}
PropPage::RStaticBox* PropPage::lockup(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* lockup = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Lockup");

  settings.lockupDelay = Misc::createNumEntry(lockup, "Lockup Delay (ms)", Misc::ID_PropOption, 0, 3000, 200);
  settings.battleModeClash = Misc::createNumEntryDouble(lockup, "Battle Mode Clash/Lockup Threshold", Misc::ID_PropOption, 0, 8, 4);

  lockup->Add(settings.lockupDelay->box, FIRSTITEMFLAGS);
  lockup->Add(settings.battleModeClash->box, MENUITEMFLAGS);

  return lockup;
}
PropPage::RStaticBox* PropPage::bmControls(wxStaticBoxSizer* parent) {
  PropPage::RStaticBox* bmControls = new PropPage::RStaticBox(wxVERTICAL, parent->GetStaticBox(), "Controls");

  settings.forcePushBM = new wxCheckBox(bmControls->GetStaticBox(), Misc::ID_PropOption, "Enable Force Push (BM Only)");
  settings.battleModeDisablePWR = new wxCheckBox(bmControls->GetStaticBox(), Misc::ID_PropOption, "Disable Power Button in Battle Mode");
  bmControls->Add(settings.battleModeDisablePWR, FIRSTITEMFLAGS);
  bmControls->Add(settings.forcePushBM, MENUITEMFLAGS);

  return bmControls;
}

PropPage::RStaticBox::RStaticBox(int orient, wxWindow* win, const wxString& label = wxEmptyString) : wxStaticBoxSizer(orient, win, label) {
  PropPage::instance->boxes.insert(PropPage::instance->boxes.begin(), this);
}
