#include "proppage.h"

#include "misc.h"
#include "defines.h"
#include "generalpage.h"
#include "mainwindow.h"

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

PropPage* PropPage::instance;
PropPage::PropPage(wxWindow* window) : wxScrolledWindow(window) {
  PropPage::instance = this;

  sizer = new wxStaticBoxSizer(wxVERTICAL, this, "");
  prop = new wxComboBox(sizer->GetStaticBox(), ID_Select, PR_DEFAULT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({PR_DEFAULT, PR_SA22C, PR_FETT263, PR_BC, PR_CAIWYN, PR_SHTOK}), wxCB_READONLY);

  sizer->Add(prop, MENUITEMFLAGS);
  sizer->Add(createGestures(sizer), BOXITEMFLAGS);
  sizer->Add(createControls(sizer), BOXITEMFLAGS);
  sizer->Add(createFeatures(sizer), BOXITEMFLAGS);
  sizer->Add(createBattleMode(sizer), BOXITEMFLAGS);

  bindEvents();
  createToolTips();

  SetSizerAndFit(sizer);
}

void PropPage::bindEvents() {
  auto propSelectUpdate = [&](wxCommandEvent&) {
    PropPage::instance->update();
    FULLUPDATEWINDOW;
    MainWindow::instance->SetSize(wxSize(MainWindow::instance->GetSize().GetWidth(), MainWindow::instance->GetMinHeight() + PropPage::instance->GetBestVirtualSize().GetHeight()));
    MainWindow::instance->SetMinSize(wxSize(MainWindow::instance->GetSize().GetWidth(), 350));
  };

  Bind(wxEVT_COMBOBOX, propSelectUpdate, ID_Select);
  Bind(wxEVT_CHECKBOX, propSelectUpdate, ID_Option);
  Bind(wxEVT_RADIOBUTTON, propSelectUpdate, ID_Option);
  Bind(wxEVT_SPINCTRL, propSelectUpdate, ID_Option);
  Bind(wxEVT_SPINCTRLDOUBLE, propSelectUpdate, ID_Option);
}
void PropPage::createToolTips() {
  auto stabOnTip = new wxToolTip("A quick \"stab\" motion will ignite the saber");
  stabOn->SetToolTip(stabOnTip);
  auto swingOnTip = new wxToolTip("A swing faster than the threshold will auto-ignite the saber");
  swingOn->SetToolTip(swingOnTip);
  auto thrustOnTip = new wxToolTip("A quick \"thrust\" motion will ignite the saber");
  thrustOn->SetToolTip(thrustOnTip);
  auto twistOnTip = new wxToolTip("A quick twist will ignite the saber");
  twistOn->SetToolTip(twistOnTip);
  auto twistOffTip = new wxToolTip("A quick twist will retract the saber");
  twistOff->SetToolTip(twistOffTip);

  auto swingSpeedTip = new wxToolTip("Speed required for swing to ignite saber");
  swingOnSpeed->setToolTip(swingSpeedTip);

  auto fastOnTip = new wxToolTip("Skip pre-on effects for this gesture");
  stabOnFast->SetToolTip(fastOnTip);
  swingOnFast->SetToolTip(fastOnTip);
  thrustOnFast->SetToolTip(fastOnTip);
  twistOnFast->SetToolTip(fastOnTip);
  auto preonTip = new wxToolTip("Enable pre-on effects for this gesture");
  stabOnPreon->SetToolTip(preonTip);
  swingOnPreon->SetToolTip(preonTip);
  thrustOnPreon->SetToolTip(preonTip);
  twistOnPreon->SetToolTip(preonTip);
  auto fastOffTip = new wxToolTip("Skip post-off effects for this gesture");
  twistOffFast->SetToolTip(fastOffTip);
  auto postoffTip = new wxToolTip("Enable post-off effects for this gesture");
  twistOffPostoff->SetToolTip(postoffTip);
  auto noBMTip = new wxToolTip("Do not enter Battle Mode when igniting with this gesture\n(default enters Battle Mode)");
  stabOnNoBattle->SetToolTip(noBMTip);
  swingOnNoBattle->SetToolTip(noBMTip);
  thrustOnNoBattle->SetToolTip(noBMTip);
  twistOnNoBattle->SetToolTip(noBMTip);


}

void PropPage::update() {
# define SA22C propString == PR_SA22C
# define FETT263 propString == PR_FETT263
# define BC propString == PR_BC
# define SHTOK propString == PR_SHTOK
# define CAIWYN propString == PR_CAIWYN
# define ALL SA22C || FETT263 || BC || SHTOK || CAIWYN

  wxString propString = prop->GetStringSelection();

  disableGestureNoBlade->Show(BC);
  noLockupHold->Show(SA22C);
  // Stab On
  stabOn->Show(SA22C || FETT263 || BC);
  stabOnFast->Show(FETT263);
  stabOnPreon->Show(FETT263);
  stabOnNoBattle->Show(FETT263);
  // Swing On
  swingOn->Show(SA22C || FETT263 || BC);
  swingOnSpeed->box->Show(SA22C || FETT263 || BC);
  swingOnSpeed->Enable(swingOn->GetValue());
  swingOnFast->Show(FETT263);
  swingOnPreon->Show(FETT263);
  swingOnNoBattle->Show(FETT263);
  // Twist On
  twistOn->Show(SA22C || FETT263 || BC);
  twistOnFast->Show(FETT263);
  twistOnPreon->Show(FETT263);
  twistOnNoBattle->Show(FETT263);
  // Thrust On
  thrustOn->Show(SA22C || FETT263 || BC);
  thrustOnFast->Show(FETT263);
  thrustOnPreon->Show(FETT263);
  thrustOnNoBattle->Show(FETT263);
  // Twist Off
  twistOff->Show(SA22C || FETT263 || BC);
  twistOffFast->Show(FETT263);
  twistOffPostoff->Show(FETT263);

  // Battle Mode
  gestureEnBattle->Show(SA22C || BC);
  lockupDelay->box->Show(SA22C || FETT263 || BC);
  battleModeToggle->Show(FETT263);
  battleModeAlways->Show(FETT263);
  battleModeOnStart->Show(FETT263);
  battleModeDisablePWR->Show(FETT263);
  battleModeClash->box->Show(FETT263);

  // Force Push
  forcePush->Show(SA22C || FETT263 || BC);
  forcePushBM->Show(FETT263);
  forcePushLength->box->Show(SA22C || FETT263 || BC);
  if (forcePush->GetValue()) {
    forcePushBM->SetValue(true);
    forcePushBM->Disable();
  } else forcePushBM->Enable();
  if (forcePushBM->GetValue()) forcePushLength->Enable();
  else forcePushLength->num->Disable();


  // Edit Mode/Settings

  editEnable->Show(FETT263);
  editMode->Show(FETT263);
  editSettings->Show(FETT263);
  if (editEnable->GetValue()) {
    editMode->Enable();
    editSettings->Enable();
  } else {
    editMode->Disable();
    editSettings->Disable();
  }

  // Quote Player
  enableQuotePlayer->Show(FETT263);
  randomizeQuotePlayer->Show(FETT263);
  forcePlayerDefault->Show(FETT263);
  quotePlayerDefault->Show(FETT263);
  if (enableQuotePlayer->GetValue()) {
    randomizeQuotePlayer->Enable();
    forcePlayerDefault->Enable();
    quotePlayerDefault->Enable();
  } else {
    randomizeQuotePlayer->Disable();
    forcePlayerDefault->Disable();
    forcePlayerDefault->SetValue(true);
    quotePlayerDefault->Disable();
  }

  pwrClash->Show(CAIWYN);
  pwrLockup->Show(CAIWYN);
  pwrHoldOff->Show(FETT263);
  if (GeneralPage::instance->buttons->num->GetValue() == 2) pwrHoldOff->Enable();
  else pwrHoldOff->Disable();
  auxHoldLockup->Show(FETT263);
  if (GeneralPage::instance->buttons->num->GetValue() == 2) auxHoldLockup->Enable();
  else auxHoldLockup->Disable();

  meltGestureAlways->Show(FETT263);
  volumeCircular->Show(FETT263);
  brightnessCircular->Show(FETT263);
  pwrWakeGesture->Show(FETT263);

  noExtraEffects->Show(FETT263);
  specialAbilities->Show(FETT263);
  if (saveChoreo->GetValue()) {
    if (!multiPhase->GetValue()) noExtraEffects->SetValue(true);
    specialAbilities->Disable();
  } else specialAbilities->Enable();
  multiPhase->Show(FETT263);

  spinMode->Show(FETT263);
  if (auxHoldLockup->GetValue() || saveChoreo->GetValue()) spinMode->Disable();
  else spinMode->Enable();

  saveGesture->Show(FETT263);
  saveChoreo->Show(FETT263);
  dualModeSound->Show(FETT263);
  clashStrengthSound->Show(FETT263);
  clashStrengthSoundMaxClash->box->Show(FETT263);
  if (clashStrengthSound->GetValue()) clashStrengthSoundMaxClash->Enable();
  else clashStrengthSoundMaxClash->num->Disable();
  quickPresetSelect->Show(FETT263);
  spokenColors->Show(FETT263);
  spokenBatteryNone->Show(FETT263);
  spokenBatteryVolts->Show(FETT263);
  spokenBatteryPercent->Show(FETT263);

  // Disables
  beepErrors->Show(FETT263);
  trackPlayerPrompts->Show(FETT263);
  fontChangeOTF->Show(FETT263);
  styleChangeOTF->Show(FETT263);
  presetCopyOTF->Show(FETT263);
  battleModeNoToggle->Show(FETT263);
  multiBlast->Show(FETT263);
  multiBlastDisableToggle->Show(FETT263);
  if (multiBlast->GetValue()) multiBlastDisableToggle->Enable();
  else multiBlastDisableToggle->Disable();
  multiBlastSwing->Show(BC);

  for(const PropPageBox* box : boxes) {
    bool shouldShow = false;
    for (const wxWindow* item : box->GetStaticBox()->GetChildren()) {
      shouldShow = shouldShow || item->IsShown();
    }
    box->GetStaticBox()->Show(shouldShow);
  }

  SetSizer(sizer);
  FitInside();
  SetScrollbars(-1, 10, -1, 1);
  SetMinClientSize(wxSize(sizer->GetMinSize().GetWidth(), 0));

# undef SA22C
# undef CAIWYN
# undef FETT263
# undef BC
# undef SHTOK
# undef ALL
}

PropPage::PropPageBox* PropPage::createGestures(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* gesturesSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Gesture Control");
  gesturesSizer->Add(createStabOn(gesturesSizer), BOXITEMFLAGS);
  gesturesSizer->Add(createSwingOn(gesturesSizer), BOXITEMFLAGS);
  gesturesSizer->Add(createThrustOn(gesturesSizer), BOXITEMFLAGS);
  gesturesSizer->Add(createTwistOn(gesturesSizer), BOXITEMFLAGS);
  gesturesSizer->Add(createTwistOff(gesturesSizer), BOXITEMFLAGS);

  return gesturesSizer;
}
PropPage::PropPageBox* PropPage::createStabOn(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox *stabOnSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Stab On");
  stabOn = new wxCheckBox(stabOnSizer->GetStaticBox(), ID_Option, "Stab To Turn On");
  stabOnFast = new wxRadioButton(stabOnSizer->GetStaticBox(), ID_Option, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  stabOnFast->SetValue(true);
  stabOnPreon = new wxRadioButton(stabOnSizer->GetStaticBox(), ID_Option, "Enable Preon");
  stabOnNoBattle = new wxCheckBox(stabOnSizer->GetStaticBox(), ID_Option, "Do Not Activate BattleMode");
  stabOnSizer->Add(stabOn, FIRSTITEMFLAGS);
  stabOnSizer->Add(stabOnFast, MENUITEMFLAGS);
  stabOnSizer->Add(stabOnPreon, MENUITEMFLAGS);
  stabOnSizer->Add(stabOnNoBattle, MENUITEMFLAGS);

  return stabOnSizer;
}
PropPage::PropPageBox* PropPage::createSwingOn(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* swingOnSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Swing On");
  swingOn = new wxCheckBox(swingOnSizer->GetStaticBox(), ID_Option, "Swing To Turn On");
  swingOnSpeed = Misc::createNumEntry(swingOnSizer->GetStaticBox(), "Swing on Speed", ID_Option, 50, 1000, 250);
  swingOnFast = new wxRadioButton(swingOnSizer->GetStaticBox(), ID_Option, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  swingOnFast->SetValue(true);
  swingOnPreon = new wxRadioButton(swingOnSizer->GetStaticBox(), ID_Option, "Enable Preon");
  swingOnNoBattle = new wxCheckBox(swingOnSizer->GetStaticBox(), ID_Option, "Do Not Activate BattleMode");
  swingOnSizer->Add(swingOn, FIRSTITEMFLAGS);
  swingOnSizer->Add(swingOnFast, MENUITEMFLAGS);
  swingOnSizer->Add(swingOnPreon, MENUITEMFLAGS);
  swingOnSizer->Add(swingOnNoBattle, MENUITEMFLAGS);
  swingOnSizer->Add(swingOnSpeed->box, MENUITEMFLAGS);

  return swingOnSizer;
}
PropPage::PropPageBox* PropPage::createThrustOn(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox *thrustOnSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Thrust On");
  thrustOn = new wxCheckBox(thrustOnSizer->GetStaticBox(), ID_Option, "Thrust To Turn On");
  thrustOnFast = new wxRadioButton(thrustOnSizer->GetStaticBox(), ID_Option, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  thrustOnFast->SetValue(true);
  thrustOnPreon = new wxRadioButton(thrustOnSizer->GetStaticBox(), ID_Option, "Enable Preon");
  thrustOnNoBattle = new wxCheckBox(thrustOnSizer->GetStaticBox(), ID_Option, "Do Not Activate BattleMode");
  thrustOnSizer->Add(thrustOn, FIRSTITEMFLAGS);
  thrustOnSizer->Add(thrustOnFast, MENUITEMFLAGS);
  thrustOnSizer->Add(thrustOnPreon, MENUITEMFLAGS);
  thrustOnSizer->Add(thrustOnNoBattle, MENUITEMFLAGS);

  return thrustOnSizer;
}
PropPage::PropPageBox* PropPage::createTwistOn(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox *twistOnSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Twist On");
  twistOn = new wxCheckBox(twistOnSizer->GetStaticBox(), ID_Option, "Twist To Turn On");
  twistOnFast = new wxRadioButton(twistOnSizer->GetStaticBox(), ID_Option, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  twistOnFast->SetValue(true);
  twistOnPreon = new wxRadioButton(twistOnSizer->GetStaticBox(), ID_Option, "Enable Preon");
  twistOnNoBattle = new wxCheckBox(twistOnSizer->GetStaticBox(), ID_Option, "Do Not Activate BattleMode");
  twistOnSizer->Add(twistOn, FIRSTITEMFLAGS);
  twistOnSizer->Add(twistOnFast, MENUITEMFLAGS);
  twistOnSizer->Add(twistOnPreon, MENUITEMFLAGS);
  twistOnSizer->Add(twistOnNoBattle, MENUITEMFLAGS);

  return twistOnSizer;
}
PropPage::PropPageBox* PropPage::createTwistOff(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox *twistOffSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Twist Off");
  twistOff = new wxCheckBox(twistOffSizer->GetStaticBox(), ID_Option, "Twist To Turn Off");
  twistOffFast = new wxRadioButton(twistOffSizer->GetStaticBox(), ID_Option, "Fast Retraction", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  twistOffFast->SetValue(true);
  twistOffPostoff = new wxRadioButton(twistOffSizer->GetStaticBox(), ID_Option, "Enable Postoff");
  twistOffPostoff->SetValue(true);
  twistOffSizer->Add(twistOff, FIRSTITEMFLAGS);
  twistOffSizer->Add(twistOffPostoff, MENUITEMFLAGS);
  twistOffSizer->Add(twistOffFast, MENUITEMFLAGS);

  return twistOffSizer;
}

PropPage::PropPageBox* PropPage::createControls(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* controlsSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Controls");

  controlsSizer->Add(createGeneralControls(controlsSizer), BOXITEMFLAGS);
  controlsSizer->Add(createEditMode(controlsSizer), BOXITEMFLAGS);
  controlsSizer->Add(createInterfaceOptions(controlsSizer), BOXITEMFLAGS);

  return controlsSizer;
}
PropPage::PropPageBox* PropPage::createGeneralControls(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* generalControlsSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "General");
  wxBoxSizer* generalControls1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalControls2 = new wxBoxSizer(wxVERTICAL);

  pwrClash = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Press PWR to Clash");
  pwrLockup = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Hold PWR to Lockup");
  pwrHoldOff = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Hold PWR to Turn Off");
  auxHoldLockup = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Hold AUX to Lockup");
  disableGestureNoBlade = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Disable Gestures Without Blade");
  noLockupHold = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Revert Lockup and Multi-Blast Trigger");
  generalControls1->Add(noLockupHold, FIRSTITEMFLAGS);
  generalControls1->Add(disableGestureNoBlade, MENUITEMFLAGS);
  generalControls1->Add(pwrClash, MENUITEMFLAGS);
  generalControls1->Add(pwrLockup, MENUITEMFLAGS);

  generalControls1->Add(pwrHoldOff, FIRSTITEMFLAGS);
  generalControls1->Add(auxHoldLockup, MENUITEMFLAGS);

  meltGestureAlways = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Always use Melt Guesture");
  volumeCircular = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Use Circular Volume Menu");
  brightnessCircular = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "Use Circular Brightness Menu");
  pwrWakeGesture = new wxCheckBox(generalControlsSizer->GetStaticBox(), ID_Option, "PWR After Timeout Enables Gestures");
  generalControls2->Add(meltGestureAlways, FIRSTITEMFLAGS);
  generalControls2->Add(volumeCircular, MENUITEMFLAGS);
  generalControls2->Add(brightnessCircular, MENUITEMFLAGS);
  generalControls2->Add(pwrWakeGesture, MENUITEMFLAGS);

  generalControlsSizer->Add(generalControls1);
  generalControlsSizer->Add(generalControls2);

  return generalControlsSizer;
}
PropPage::PropPageBox* PropPage::createEditMode(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* editModeSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Edit Mode");


  editEnable = new wxCheckBox(editModeSizer->GetStaticBox(), ID_Option, "Enable On-The-Fly Editing");
  editMode = new wxRadioButton(editModeSizer->GetStaticBox(), ID_Option, "Enable Edit Mode", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  editMode->SetValue(true);
  editSettings = new wxRadioButton(editModeSizer->GetStaticBox(), ID_Option, "Enable Edit Settings");
  editModeSizer->Add(editEnable, FIRSTITEMFLAGS);
  editModeSizer->Add(editMode, MENUITEMFLAGS);
  editModeSizer->Add(editSettings, MENUITEMFLAGS);

  return editModeSizer;
}
PropPage::PropPageBox* PropPage::createInterfaceOptions(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* interfaceSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Interface");
  wxBoxSizer* interface1 = new wxBoxSizer(wxVERTICAL);

  beepErrors = new wxCheckBox(interfaceSizer->GetStaticBox(), ID_Option, "Beep Errors Instead of Spoken");
  trackPlayerPrompts = new wxCheckBox(interfaceSizer->GetStaticBox(), ID_Option, "Enable Track Player Prompts");
  trackPlayerPrompts->SetValue(true);
  spokenColors = new wxCheckBox(interfaceSizer->GetStaticBox(), ID_Option, "Enable Spoken Colors");
  spokenBatteryNone = new wxRadioButton(interfaceSizer->GetStaticBox(), ID_Option, "Battery Speak None", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  spokenBatteryNone->SetValue(true);
  spokenBatteryVolts = new wxRadioButton(interfaceSizer->GetStaticBox(), ID_Option, "Battery Speak Voltage");
  spokenBatteryPercent = new wxRadioButton(interfaceSizer->GetStaticBox(), ID_Option, "Battery Speak Percentage");
  interface1->Add(beepErrors, FIRSTITEMFLAGS);
  interface1->Add(trackPlayerPrompts, MENUITEMFLAGS);
  interface1->Add(spokenColors, MENUITEMFLAGS);
  interface1->Add(spokenBatteryNone, MENUITEMFLAGS);
  interface1->Add(spokenBatteryVolts, MENUITEMFLAGS);
  interface1->Add(spokenBatteryPercent, MENUITEMFLAGS);

  interfaceSizer->Add(interface1);

  return interfaceSizer;
}

PropPage::PropPageBox* PropPage::createFeatures(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* featuresSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Features");

  featuresSizer->Add(createForcePush(featuresSizer), BOXITEMFLAGS);
  featuresSizer->Add(createQuotePlayer(featuresSizer), BOXITEMFLAGS);
  featuresSizer->Add(createGeneralFeatures(featuresSizer), BOXITEMFLAGS);

  return featuresSizer;
}
PropPage::PropPageBox* PropPage::createForcePush(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* forcePushSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Force Push");

  forcePush = new wxCheckBox(forcePushSizer->GetStaticBox(), ID_Option, "Enable Force Push");
  forcePushLength = Misc::createNumEntry(forcePushSizer->GetStaticBox(), "Force Push Length", ID_Option, 0, 10, 5);
  forcePushSizer->Add(forcePush, FIRSTITEMFLAGS);
  forcePushSizer->Add(forcePushLength->box, MENUITEMFLAGS);

  return forcePushSizer;
}
PropPage::PropPageBox* PropPage::createQuotePlayer(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* quotePlayerSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Quote Player");

  enableQuotePlayer = new wxCheckBox(quotePlayerSizer->GetStaticBox(), ID_Option, "Enable Quote Player");
  enableQuotePlayer->SetValue(true);
  randomizeQuotePlayer = new wxCheckBox(quotePlayerSizer->GetStaticBox(), ID_Option, "Randomize Quotes");
  forcePlayerDefault = new wxRadioButton(quotePlayerSizer->GetStaticBox(), ID_Option, "Make Force FX Default");
  forcePlayerDefault->SetValue(true);
  quotePlayerDefault = new wxRadioButton(quotePlayerSizer->GetStaticBox(), ID_Option, "Make Quotes Default");

  quotePlayerSizer->Add(enableQuotePlayer, FIRSTITEMFLAGS);
  quotePlayerSizer->Add(randomizeQuotePlayer, MENUITEMFLAGS);
  quotePlayerSizer->Add(forcePlayerDefault, MENUITEMFLAGS);
  quotePlayerSizer->Add(quotePlayerDefault, MENUITEMFLAGS);

  return quotePlayerSizer;
}
PropPage::PropPageBox* PropPage::createGeneralFeatures(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* generalFeaturesSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Other");
  wxBoxSizer* generalFeatures1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalFeatures2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* generalFeatures3 = new wxBoxSizer(wxVERTICAL);

  noExtraEffects = new wxRadioButton(generalFeaturesSizer->GetStaticBox(), ID_Option, "No Extra Effects", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  noExtraEffects->SetValue(true);
  specialAbilities = new wxRadioButton(generalFeaturesSizer->GetStaticBox(), ID_Option, "Special Abilities");
  multiPhase = new wxRadioButton(generalFeaturesSizer->GetStaticBox(), ID_Option, "Multi-Phase Styles");

  spinMode = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Toggle for Spin Mode");
  saveChoreo = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Choreography");

  fontChangeOTF = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "OTF Font Change");
  fontChangeOTF->SetValue(true);
  styleChangeOTF = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "OTF Style Change");
  styleChangeOTF->SetValue(true);
  presetCopyOTF = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "OTF Preset Copying");
  presetCopyOTF->SetValue(true);

  saveGesture = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Save \"Disable Gesture\"");
  dualModeSound = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Ignition Sound Angle");
  clashStrengthSound = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Clash Sound Strength");
  clashStrengthSoundMaxClash = Misc::createNumEntry(generalFeaturesSizer->GetStaticBox(), "CSS Max Clash", ID_Option, 8, 16, 10);
  quickPresetSelect = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Preset Select on Boot");

  multiBlast = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Multi-Blast");
  multiBlast->SetValue(true);
  multiBlastDisableToggle = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Multi-Blast Guesture Only");
  multiBlastSwing = new wxCheckBox(generalFeaturesSizer->GetStaticBox(), ID_Option, "Multi-Blast On Swing");

  generalFeatures1->Add(noExtraEffects, MENUITEMFLAGS);
  generalFeatures1->Add(specialAbilities, MENUITEMFLAGS);
  generalFeatures1->Add(multiPhase, MENUITEMFLAGS);
  generalFeatures1->Add(spinMode, MENUITEMFLAGS);
  generalFeatures1->Add(saveChoreo, MENUITEMFLAGS);

  generalFeatures2->Add(saveGesture, MENUITEMFLAGS);
  generalFeatures2->Add(dualModeSound, MENUITEMFLAGS);
  generalFeatures2->Add(quickPresetSelect, MENUITEMFLAGS);
  generalFeatures2->Add(multiBlast, MENUITEMFLAGS);
  generalFeatures2->Add(multiBlastDisableToggle, MENUITEMFLAGS);
  generalFeatures2->Add(multiBlastSwing, MENUITEMFLAGS);

  generalFeatures3->Add(fontChangeOTF, MENUITEMFLAGS);
  generalFeatures3->Add(styleChangeOTF, MENUITEMFLAGS);
  generalFeatures3->Add(presetCopyOTF, MENUITEMFLAGS);
  generalFeatures3->Add(clashStrengthSound, MENUITEMFLAGS);
  generalFeatures3->Add(clashStrengthSoundMaxClash->box, MENUITEMFLAGS);

  generalFeaturesSizer->Add(generalFeatures1);
  generalFeaturesSizer->Add(generalFeatures2);
  generalFeaturesSizer->Add(generalFeatures3);

  return generalFeaturesSizer;
}

PropPage::PropPageBox* PropPage::createBattleMode(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* battleModeSizer = new PropPage::PropPageBox(wxHORIZONTAL, parent->GetStaticBox(), "Battle Mode");

  battleModeSizer->Add(createActivation(battleModeSizer), BOXITEMFLAGS);
  battleModeSizer->Add(createLockup(battleModeSizer), BOXITEMFLAGS);
  battleModeSizer->Add(createBMControls(battleModeSizer), BOXITEMFLAGS);

  return battleModeSizer;
}
PropPage::PropPageBox* PropPage::createActivation(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* activationSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Activation");

  battleModeToggle = new wxRadioButton(activationSizer->GetStaticBox(), ID_Option, "Battle Mode Toggle On", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  battleModeToggle->SetValue(true);
  battleModeAlways = new wxRadioButton(activationSizer->GetStaticBox(), ID_Option, "Battle Mode Always On");
  battleModeOnStart = new wxRadioButton(activationSizer->GetStaticBox(), ID_Option, "Battle Mode On Start");

  battleModeNoToggle = new wxCheckBox(activationSizer->GetStaticBox(), ID_Option, "Battle Mode Only On Gesture");

  gestureEnBattle = new wxCheckBox(activationSizer->GetStaticBox(), ID_Option, "Gesture Ignition Starts Battle Mode");

  activationSizer->Add(battleModeToggle, FIRSTITEMFLAGS);
  activationSizer->Add(battleModeOnStart, MENUITEMFLAGS);
  activationSizer->Add(battleModeAlways, MENUITEMFLAGS);

  activationSizer->Add(battleModeNoToggle, FIRSTITEMFLAGS);

  activationSizer->Add(gestureEnBattle, FIRSTITEMFLAGS);

  return activationSizer;
}
PropPage::PropPageBox* PropPage::createLockup(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* lockupSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Lockup");

  lockupDelay = Misc::createNumEntry(lockupSizer->GetStaticBox(), "Lockup Delay (ms)", ID_Option, 0, 3000, 200);
  battleModeClash = Misc::createNumEntryDouble(lockupSizer->GetStaticBox(), "Battle Mode Clash/Lockup Threshold", ID_Option, 0, 8, 4);

  lockupSizer->Add(lockupDelay->box, FIRSTITEMFLAGS);
  lockupSizer->Add(battleModeClash->box, MENUITEMFLAGS);

  return lockupSizer;
}
PropPage::PropPageBox* PropPage::createBMControls(wxStaticBoxSizer* parent) {
  PropPage::PropPageBox* bmControlsSizer = new PropPage::PropPageBox(wxVERTICAL, parent->GetStaticBox(), "Controls");

  forcePushBM = new wxCheckBox(bmControlsSizer->GetStaticBox(), ID_Option, "Enable Force Push (BM Only)");
  battleModeDisablePWR = new wxCheckBox(bmControlsSizer->GetStaticBox(), ID_Option, "Disable Power Button in Battle Mode");
  bmControlsSizer->Add(battleModeDisablePWR, FIRSTITEMFLAGS);
  bmControlsSizer->Add(forcePushBM, MENUITEMFLAGS);

  return bmControlsSizer;
}



PropPage::PropPageBox::PropPageBox(int32_t orient, wxWindow* win, const wxString& label = wxEmptyString) : wxStaticBoxSizer(orient, win, label) {
  PropPage::instance->boxes.insert(PropPage::instance->boxes.begin(), this);
}
