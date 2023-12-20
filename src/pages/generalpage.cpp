// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "pages/generalpage.h"

#include "core/defines.h"

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/tooltip.h>

GeneralPage* GeneralPage::instance;
GeneralPage::GeneralPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "")
{
  Add(boardSettings(this), BOXITEMFLAGS);
  Add(optionSettings(this), BOXITEMFLAGS);

  createToolTips();
}

void GeneralPage::createToolTips() {
  TIP(board, "The version of proffieboard.");
  TIP(massStorage, "Enable to access the contents of your proffieboard's SD card via the USB connection.");
  TIP(webUSB, "Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info.");

  TIP(buttons, "Number of buttons your saber has.\nPlease note not all prop files support all possible numbers of buttons, and controls will changed depending on how many buttons are specified.");
  TIP(volume, "Maximum volume level.\n2000 is a good starting value for most speakers, and it is not recommended to go up to or past 3000 unless you know what you are doing, as this can damage your speaker.");
  TIP(clash, "Force required to trigger a clash effect.\nMeasured in Gs.");
  TIP(pliTime, "Time since last activity before PLI goes to sleep.");
  TIP(idleTime, "Time since last activity before accent LEDs go to sleep.");
  TIP(motionTime, "Time since last activity before gesture controls are disabled.");
  TIP(maxLEDs, "Maximum number of LEDs in a WS281X blade.\nThis value should not be changed unless you know what you are doing.\nConfigure the length of your blade in the \"Blade Arrays\" page.");

  TIP(volumeSave, "Save the volume level between board restarts.");
  TIP(presetSave, "Save the currently-selected preset between board restarts.");
  TIP(colorSave, "Save color edits to presets.");

  TIP(enableOLED, "Enable if you have an OLED/SSD1306 display connected.");
  TIP(disableColor, "Disable color change controls.");
  TIP(noTalkie, "Use beeps for errors instead of spoken errors and can be used to save some memory.\nSee the POD page \"What is it beeping?\".");
  TIP(noBasicParsers, "Disable basic styles for use in the ProffieOS Workbench.\nThis can be used to save memory.");
  TIP(disableDiagnosticCommands, "Disable diagnostic commands in the Serial Monitor.\nThis can be used to save memory.");
  TIP(enableDeveloperCommands, "You should not enable this unless you know what you are doing.\nEnable specialty developer commands in the Serial Monitor.");
}

wxStaticBoxSizer* GeneralPage::boardSettings(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* boardSetup = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Board Setup");

  board = new wxComboBox(boardSetup->GetStaticBox(), wxID_ANY, "ProffieBoard V2", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}), wxCB_READONLY);
  massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
  webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

  boardSetup->Add(board, FIRSTITEMFLAGS);
  boardSetup->Add(massStorage, MENUITEMFLAGS);
  boardSetup->Add(webUSB, MENUITEMFLAGS);

  return boardSetup;
}
wxStaticBoxSizer* GeneralPage::optionSettings(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), "Options");

  options->Add(numOptions(options), FIRSTITEMFLAGS);
  options->Add(boolOptions(options), FIRSTITEMFLAGS);

  return options;
}
wxBoxSizer* GeneralPage::boolOptions(wxStaticBoxSizer* parent) {
  wxBoxSizer* boolOptions = new wxBoxSizer(wxVERTICAL);

  volumeSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Volume");
  presetSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Preset");
  colorSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Color");
  enableOLED = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Enable OLED");
  disableColor = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Color Change");
  noTalkie = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Talkie");
  noBasicParsers = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Basic Parser Styles");
  disableDiagnosticCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Diagnostic Commands");
  enableDeveloperCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Enable Developer Commands");

  boolOptions->Add(volumeSave, FIRSTITEMFLAGS);
  boolOptions->Add(presetSave, MENUITEMFLAGS);
  boolOptions->Add(colorSave, MENUITEMFLAGS);
  boolOptions->Add(enableOLED, MENUITEMFLAGS);
  boolOptions->Add(disableColor, MENUITEMFLAGS);
  boolOptions->Add(noTalkie, MENUITEMFLAGS);
  boolOptions->Add(noBasicParsers, MENUITEMFLAGS);
  boolOptions->Add(disableDiagnosticCommands, MENUITEMFLAGS);
  boolOptions->Add(enableDeveloperCommands, MENUITEMFLAGS);

  return boolOptions;
}
wxBoxSizer* GeneralPage::numOptions(wxStaticBoxSizer* parent) {
  wxBoxSizer* numOptions = new wxBoxSizer(wxVERTICAL);

  buttons = Misc::createNumEntry(parent->GetStaticBox(), "Number of Buttons", wxID_ANY, 0, 3, 2);
  volume = Misc::createNumEntry(parent->GetStaticBox(), "Max Volume", wxID_ANY, 0, 3500, 2000);
  volume->num->SetIncrement(50);
  clash = Misc::createNumEntryDouble(parent->GetStaticBox(), "Clash Threshold", wxID_ANY, 0.1, 5, 3);
  pliTime = Misc::createNumEntry(parent->GetStaticBox(), "PLI Timeout", wxID_ANY, 1, 60, 2);
  idleTime = Misc::createNumEntry(parent->GetStaticBox(), "Idle Timeout", wxID_ANY, 1, 60, 10);
  motionTime = Misc::createNumEntry(parent->GetStaticBox(), "Motion Timeout", wxID_ANY, 1, 60, 15);
  maxLEDs = Misc::createNumEntry(parent->GetStaticBox(), "WS281X Max LEDs", wxID_ANY, 0, 1024, 144);

  numOptions->Add(buttons->box, FIRSTITEMFLAGS);
  numOptions->Add(volume->box, MENUITEMFLAGS);
  numOptions->Add(clash->box, MENUITEMFLAGS);
  numOptions->Add(pliTime->box, MENUITEMFLAGS);
  numOptions->Add(idleTime->box, MENUITEMFLAGS);
  numOptions->Add(motionTime->box, MENUITEMFLAGS);
  numOptions->Add(maxLEDs->box, MENUITEMFLAGS);

  return numOptions;
}
