// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/pages/generalpage.h"

#include "core/defines.h"
#include "core/utilities/misc.h"

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

  TIP(buttons->entry(), "Number of buttons your saber has.\nPlease note not all prop files support all possible numbers of buttons, and controls will changed depending on how many buttons are specified.");
  TIP(volume->entry(), "Maximum volume level.\n1500 is a good starting value for most speakers, and it is not recommended to go past 2000 unless you know what you are doing, as this can damage your speaker.");
  TIP(clash->entry(), "Force required to trigger a clash effect.\nMeasured in Gs.");
  TIP(pliTime->entry(), "Time since last activity before PLI goes to sleep.");
  TIP(idleTime->entry(), "Time since last activity before accent LEDs go to sleep.");
  TIP(motionTime->entry(), "Time since last activity before gesture controls are disabled.");
  TIP(maxLEDs->entry(), "Maximum number of LEDs in a WS281X blade.\nThis value should not be changed unless you know what you are doing.\nConfigure the length of your blade in the \"Blade Arrays\" page.");

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

  board = new pcComboBox(boardSetup->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}), wxCB_READONLY);
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

  buttons = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Number of Buttons", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3, 2, wxHORIZONTAL);
  volume = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Max Volume", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 5000, 1500, wxHORIZONTAL);
  volume->entry()->SetIncrement(50);
  clash = new pcSpinCtrlDouble(parent->GetStaticBox(), wxID_ANY, "Clash Threshold", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.1, 5, 3, wxHORIZONTAL);
  pliTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "PLI Timeout", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 2, wxHORIZONTAL);
  idleTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Idle Timeout", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 10, wxHORIZONTAL);
  motionTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Motion Timeout", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 15, wxHORIZONTAL);
  maxLEDs = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "WS281X Max LEDs", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1024, 144, wxHORIZONTAL);

  numOptions->Add(buttons, FIRSTITEMFLAGS);
  numOptions->Add(volume, MENUITEMFLAGS);
  numOptions->Add(clash, MENUITEMFLAGS);
  numOptions->Add(pliTime, MENUITEMFLAGS);
  numOptions->Add(idleTime, MENUITEMFLAGS);
  numOptions->Add(motionTime, MENUITEMFLAGS);
  numOptions->Add(maxLEDs, MENUITEMFLAGS);

  return numOptions;
}
