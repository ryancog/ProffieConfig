#include "generalpage.h"
#include "defines.h"
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

void GeneralPage::createBoardSettings() {
  settings.board = new wxComboBox(boardSetup->GetStaticBox(), wxID_ANY, "ProffieBoard V2", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}), wxCB_READONLY);
  settings.massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
  settings.webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

  boardSetup->Add(settings.board, wxSizerFlags(0).Border(wxALL, 10));
  boardSetup->Add(settings.massStorage, wxSizerFlags(0).Border(wxALL, 10));
  boardSetup->Add(settings.webUSB, wxSizerFlags(0).Border(wxALL, 10));
}

void GeneralPage::createPropSettings() {
  settings.prop = new wxComboBox(propSetup->GetStaticBox(), Misc::ID_PropSelect, PR_SA22C, wxDefaultPosition, wxDefaultSize, Misc::createEntries({PR_DEFAULT, PR_SA22C, PR_FETT263, PR_SHTOK, PR_BC}), wxCB_READONLY);

  wxWrapSizer *propSetup2 = new wxWrapSizer(wxVERTICAL, 0);
  wxStaticBoxSizer *battleMode = new wxStaticBoxSizer(wxVERTICAL, propSetup->GetStaticBox(), "Battle Mode");

  //settings.disableGuestureNoBlade = new wxCheckBox(propSetup->GetStaticBox(), Misc::ID_PropOption, "Disable Guestures without Blade");
  settings.noLockupHold = new wxCheckBox(propSetup->GetStaticBox(), Misc::ID_PropOption, "Revert Lockup and Multi-Blast Trigger");

  // Battle Mode
  settings.gestureEnBattle = new wxCheckBox(battleMode->GetStaticBox(), Misc::ID_PropOption, "Gesture Ignition Starts Battle Mode");
  settings.lockupDelay = Misc::createNumEntry(battleMode, "Lockup Delay (ms)", Misc::ID_PropOption, 0, 3000, 200);
  //settings.battleModeToggle->Show(FETT263);
  //settings.battleModeAlways->Show(FETT263);
  //settings.battleModeOnStart->Show(FETT263);
  //settings.battleModeDisablePWR->Show(FETT263);
  //settings.battleModeClash.num->Show(FETT263);

  // Force Push

  //settings.forcePushAlways->Show(FETT263);

  /*
        // Edit Mode/Settings
        settings.editMode->Show(FETT263);
        if (!settings.editSettings->GetValue()) settings.editMode->Enable();
        else settings.editMode->Disable();
        settings.editSettings->Show(FETT263);
        if (!settings.editMode->GetValue()) settings.editSettings->Enable();
        else settings.editSettings->Disable();

        // Quote Player
        settings.enableQuotePlayer->Show(FETT263);
        settings.randomizeQuotePlayer->Show(FETT263);
        settings.forcePlayerDefault->Show(FETT263);
        settings.quotePlayerDefault->Show(FETT263);

        settings.pwrClash->Show(CAIWYN);
        settings.pwrLockup->Show(CAIWYN);
        settings.pwrHoldOff->Show(FETT263);
        if (settings.buttons.num->GetValue() == 2) settings.pwrHoldOff->Enable();
        else settings.pwrHoldOff->Disable();
        settings.auxHoldLockup->Show(FETT263);
        if (settings.buttons.num->GetValue() == 2) settings.auxHoldLockup->Enable();
        else settings.auxHoldLockup->Disable();

        settings.meltGuestureAlways->Show(FETT263);
        settings.volumeCircular->Show(FETT263);
        settings.brightnessCircular->Show(FETT263);
        settings.pwrWakeGuesture->Show(FETT263);
        settings.specialAbilities->Show(FETT263);
        if (settings.multiPhase->GetValue() || settings.saveChoreo->GetValue()) settings.specialAbilities->Disable();
        else settings.specialAbilities->Enable();

        settings.multiPhase->Show(FETT263);
        settings.spinMode->Show(FETT263);
        if (settings.pwrLockup->GetValue() || settings.saveChoreo->GetValue()) settings.spinMode->Disable();
        else settings.spinMode->Enable();

        settings.saveGuestureDisable->Show(FETT263);
        settings.saveChoreo->Show(FETT263);
        settings.dualModeSound->Show(FETT263);
        settings.clashStrengthSound->Show(FETT263);
        settings.maxClash.num->Show(FETT263);
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
        settings.battleToggle->Show(FETT263);
        settings.multiBlast->Show(FETT263);
        settings.multiBlastSwing->Show(BC);
        settings.multiBlastToggle->Show(FETT263);
        */


  // Prop Setup 2
  //propSetup2->Add(settings.disableGuestureNoBlade, MENUITEMFLAGS);
  propSetup2->Add(settings.noLockupHold, MENUITEMFLAGS);
  // Battle Mode
  battleMode->Add(settings.gestureEnBattle, MENUITEMFLAGS);
  battleMode->Add(settings.lockupDelay.box, MENUITEMFLAGS);
  propSetup2->Add(battleMode, wxSizerFlags(0).Border(wxALL, 10));

  propSetup->Add(settings.prop, MENUITEMFLAGS);
  propSetup->Add(guestures(propSetup), MENUITEMFLAGS.Expand());
  propSetup->Add(controls(propSetup), MENUITEMFLAGS.Expand());

}

GeneralPage::GeneralPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "General")
{
  generalHoriz = new wxBoxSizer(wxHORIZONTAL);
  boardSetup = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), "Board Setup");
  propSetup = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), "Prop Setup");
  options = new wxStaticBoxSizer(wxHORIZONTAL, GetStaticBox(), "Options");


  wxWrapSizer *options1 = new wxWrapSizer(wxVERTICAL, 0);
  wxWrapSizer *options2 = new wxWrapSizer(wxVERTICAL, 0);

  createBoardSettings();
  createPropSettings();

  // Options
  {
    settings.buttons = Misc::createNumEntry(options, "Number of Buttons", wxID_ANY, 1, 3, 2);
    settings.volume = Misc::createNumEntry(options, "Max Volume", wxID_ANY, 0, 3500, 2000);
    settings.volume.num->SetIncrement(50);
    settings.clash = Misc::createNumEntryDouble(options, "Clash Threshold", wxID_ANY, 0.1, 5, 3);
    settings.pliTime = Misc::createNumEntry(options, "PLI Timeout", wxID_ANY, 1, 60, 2);
    settings.idleTime = Misc::createNumEntry(options, "Idle Timeout", wxID_ANY, 1, 60, 10);
    settings.motion = Misc::createNumEntry(options, "Motion Timeout", wxID_ANY, 1, 60, 15);

    settings.volumeSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Volume");
    settings.presetSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Preset");
    settings.colorSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Color");
    settings.disableColor = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Color Change");
    settings.disableDev = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Developer Commands");

    // Options 1
    options1->Add(settings.buttons.box, MENUITEMFLAGS);
    options1->Add(settings.volume.box, MENUITEMFLAGS);
    options1->Add(settings.clash.box, MENUITEMFLAGS);
    options1->Add(settings.pliTime.box, MENUITEMFLAGS);
    options1->Add(settings.idleTime.box, MENUITEMFLAGS);
    options1->Add(settings.motion.box, MENUITEMFLAGS);
    options2->Add(settings.volumeSave, MENUITEMFLAGS);
    options2->Add(settings.presetSave, MENUITEMFLAGS);
    options2->Add(settings.colorSave, MENUITEMFLAGS);
    options2->Add(settings.disableColor, MENUITEMFLAGS);
    options2->Add(settings.disableDev, MENUITEMFLAGS);

    options->Add(options1);
    options->Add(options2);
  }

  generalHoriz->Add(boardSetup, wxSizerFlags(/*proportion*/ 2).Border(wxALL, 10).Expand());
  generalHoriz->Add(propSetup, wxSizerFlags(/*proportion*/ 7).Border(wxALL, 10).Expand());

  Add(generalHoriz, wxSizerFlags(10).Expand());
  Add(options, wxSizerFlags(8).Border(wxALL, 10).Expand());
}

void GeneralPage::updatePropOptions() {
  std::string prop = settings.prop->GetStringSelection().ToStdString();
#define SA22C prop == PR_SA22C
#define FETT263 prop == PR_FETT263
#define BC prop == PR_BC
#define SHTOK prop == PR_SHTOK
#define CAIWYN prop == PR_CAIWYN
#define ALL SA22C || FETT263 || BC || SHTOK || CAIWYN


  settings.disableGuestureNoBlade->Show(BC);
  settings.noLockupHold->Show(SA22C);
  // Stab On
  settings.stabOn->Show(SA22C || FETT263 || BC);
  settings.stabOnFast->Show(FETT263);
  settings.stabOnPreon->Show(FETT263);
  settings.stabOnNoBattle->Show(FETT263);
  // Swing On
  settings.swingOn->Show(SA22C || FETT263 || BC);
  settings.swingOnSpeed.num->Show(SA22C || FETT263);
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
  settings.lockupDelay.num->Show(SA22C || FETT263 || BC);
  settings.battleModeToggle->Show(FETT263);
  settings.battleModeAlways->Show(FETT263);
  settings.battleModeOnStart->Show(FETT263);
  settings.battleModeDisablePWR->Show(FETT263);
  settings.battleModeClash.num->Show(FETT263);

  // Force Push
  settings.forcePush->Show(SA22C || FETT263 || BC);
  settings.forcePushAlways->Show(FETT263);
  settings.forcePushLength.num->Show(SA22C || FETT263 || BC);

  // Edit Mode/Settings
  settings.editMode->Show(FETT263);
  if (!settings.editSettings->GetValue()) settings.editMode->Enable();
  else settings.editMode->Disable();
  settings.editSettings->Show(FETT263);
  if (!settings.editMode->GetValue()) settings.editSettings->Enable();
  else settings.editSettings->Disable();

  // Quote Player
  settings.enableQuotePlayer->Show(FETT263);
  settings.randomizeQuotePlayer->Show(FETT263);
  settings.forcePlayerDefault->Show(FETT263);
  settings.quotePlayerDefault->Show(FETT263);

  settings.pwrClash->Show(CAIWYN);
  settings.pwrLockup->Show(CAIWYN);
  settings.pwrHoldOff->Show(FETT263);
  if (settings.buttons.num->GetValue() == 2) settings.pwrHoldOff->Enable();
  else settings.pwrHoldOff->Disable();
  settings.auxHoldLockup->Show(FETT263);
  if (settings.buttons.num->GetValue() == 2) settings.auxHoldLockup->Enable();
  else settings.auxHoldLockup->Disable();

  settings.meltGuestureAlways->Show(FETT263);
  settings.volumeCircular->Show(FETT263);
  settings.brightnessCircular->Show(FETT263);
  settings.pwrWakeGuesture->Show(FETT263);
  settings.specialAbilities->Show(FETT263);
  if (settings.multiPhase->GetValue() || settings.saveChoreo->GetValue()) settings.specialAbilities->Disable();
  else settings.specialAbilities->Enable();

  settings.multiPhase->Show(FETT263);
  settings.spinMode->Show(FETT263);
  if (settings.pwrLockup->GetValue() || settings.saveChoreo->GetValue()) settings.spinMode->Disable();
  else settings.spinMode->Enable();

  settings.saveGuestureDisable->Show(FETT263);
  settings.saveChoreo->Show(FETT263);
  settings.dualModeSound->Show(FETT263);
  settings.clashStrengthSound->Show(FETT263);
  settings.maxClash.num->Show(FETT263);
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
  settings.battleToggle->Show(FETT263);
  settings.multiBlast->Show(FETT263);
  settings.multiBlastSwing->Show(BC);
  settings.multiBlastToggle->Show(FETT263);

#undef SA22C
#undef FETT263
#undef BC
#undef SHTOK
#undef ALL
}

wxStaticBoxSizer* GeneralPage::guestures(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* guestures = new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), "Guesture Control");

  guestures->Add(stabOn(guestures), MENUITEMFLAGS.Expand());
  guestures->Add(swingOn(guestures), MENUITEMFLAGS.Expand());
  guestures->Add(thrustOn(guestures), MENUITEMFLAGS.Expand());
  guestures->Add(twistOn(guestures), MENUITEMFLAGS.Expand());
  guestures->Add(twistOff(guestures), MENUITEMFLAGS.Expand());

  return guestures;
}
wxStaticBoxSizer* GeneralPage::stabOn(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *stabOn = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Stab On");
  settings.stabOn = new wxCheckBox(stabOn->GetStaticBox(), Misc::ID_PropOption, "Stab To Turn On");
  settings.stabOnFast = new wxRadioButton(stabOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.stabOnPreon = new wxRadioButton(stabOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.stabOnNoBattle = new wxCheckBox(stabOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  stabOn->Add(settings.stabOn, MENUITEMFLAGS);
  stabOn->Add(settings.stabOnFast, MENUITEMFLAGS);
  stabOn->Add(settings.stabOnPreon, MENUITEMFLAGS);
  stabOn->Add(settings.stabOnNoBattle, MENUITEMFLAGS);

  return stabOn;
}
wxStaticBoxSizer* GeneralPage::swingOn(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *swingOn = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Swing On");
  settings.swingOn = new wxCheckBox(swingOn->GetStaticBox(), Misc::ID_PropOption, "Swing To Turn On");
  settings.swingOnSpeed = Misc::createNumEntry(swingOn, "Swing on Speed", Misc::ID_PropOption, 50, 1000, 250);
  settings.swingOnFast = new wxRadioButton(swingOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.swingOnPreon = new wxRadioButton(swingOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.swingOnNoBattle = new wxCheckBox(swingOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  swingOn->Add(settings.swingOn, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnFast, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnPreon, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnNoBattle, MENUITEMFLAGS);
  swingOn->Add(settings.swingOnSpeed.box, MENUITEMFLAGS);

  return swingOn;
}
wxStaticBoxSizer* GeneralPage::thrustOn(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *thrustOn = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Thrust On");
  settings.thrustOn = new wxCheckBox(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Thrust To Turn On");
  settings.thrustOnFast = new wxRadioButton(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.thrustOnPreon = new wxRadioButton(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.thrustOnNoBattle = new wxCheckBox(thrustOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  thrustOn->Add(settings.thrustOn, MENUITEMFLAGS);
  thrustOn->Add(settings.thrustOnFast, MENUITEMFLAGS);
  thrustOn->Add(settings.thrustOnPreon, MENUITEMFLAGS);
  thrustOn->Add(settings.thrustOnNoBattle, MENUITEMFLAGS);

  return thrustOn;
}
wxStaticBoxSizer* GeneralPage::twistOn(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *twistOn = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Twist On");
  settings.twistOn = new wxCheckBox(twistOn->GetStaticBox(), Misc::ID_PropOption, "Twist To Turn On");
  settings.twistOnFast = new wxRadioButton(twistOn->GetStaticBox(), Misc::ID_PropOption, "Fast Ignition", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.twistOnPreon = new wxRadioButton(twistOn->GetStaticBox(), Misc::ID_PropOption, "Enable Preon");
  settings.twistOnNoBattle = new wxCheckBox(twistOn->GetStaticBox(), Misc::ID_PropOption, "Do Not Activate BattleMode");
  twistOn->Add(settings.twistOn, MENUITEMFLAGS);
  twistOn->Add(settings.twistOnFast, MENUITEMFLAGS);
  twistOn->Add(settings.twistOnPreon, MENUITEMFLAGS);
  twistOn->Add(settings.twistOnNoBattle, MENUITEMFLAGS);

  return twistOn;
}
wxStaticBoxSizer* GeneralPage::twistOff(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *twistOff = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Twist Off");
  settings.twistOff = new wxCheckBox(twistOff->GetStaticBox(), Misc::ID_PropOption, "Twist To Turn Off");
  settings.twistOffFast = new wxRadioButton(twistOff->GetStaticBox(), Misc::ID_PropOption, "Fast Retraction", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  settings.twistOffPostoff = new wxRadioButton(twistOff->GetStaticBox(), Misc::ID_PropOption, "Enable Postoff");
  twistOff->Add(settings.twistOff, MENUITEMFLAGS);
  twistOff->Add(settings.twistOffFast, MENUITEMFLAGS);
  twistOff->Add(settings.twistOffPostoff, MENUITEMFLAGS);

  return twistOff;
}

wxStaticBoxSizer* GeneralPage::controls(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* controls = new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), "Controls");

  controls->Add(forcePush(controls), MENUITEMFLAGS);

  return controls;
}
wxStaticBoxSizer* GeneralPage::battleMode(wxStaticBoxSizer* parent) {

}
wxStaticBoxSizer* GeneralPage::forcePush(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* forcePush = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Force Push");

  settings.forcePush = new wxCheckBox(forcePush->GetStaticBox(), wxID_ANY, "Enable Force Push");
  settings.forcePushLength = Misc::createNumEntry(forcePush, "Force Push Length", wxID_ANY, 0, 10, 5);
  forcePush->Add(settings.forcePush, MENUITEMFLAGS);
  forcePush->Add(settings.forcePushLength.box, MENUITEMFLAGS);

  return forcePush;
}


decltype(GeneralPage::settings) GeneralPage::settings;
