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

GeneralPage* GeneralPage::instance;
GeneralPage::GeneralPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "")
{
  Add(boardSettings(this), BOXITEMFLAGS);
  Add(optionSettings(this), BOXITEMFLAGS);
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
  disableColor = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Color Change");
  noTalkie = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Talkie");
  noBasicParsers = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Basic Parser Styles");
  disableDiagnosticCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Diagnostic Commands");
  enableDeveloperCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Enable Developer Commands");
  maxLEDs = Misc::createNumEntry(parent, "WS281X Max LEDs", wxID_ANY, 0, 1024, 144);

  boolOptions->Add(volumeSave, FIRSTITEMFLAGS);
  boolOptions->Add(presetSave, MENUITEMFLAGS);
  boolOptions->Add(colorSave, MENUITEMFLAGS);
  boolOptions->Add(disableColor, MENUITEMFLAGS);
  boolOptions->Add(noTalkie, MENUITEMFLAGS);
  boolOptions->Add(noBasicParsers, MENUITEMFLAGS);
  boolOptions->Add(disableDiagnosticCommands, MENUITEMFLAGS);
  boolOptions->Add(enableDeveloperCommands, MENUITEMFLAGS);
  boolOptions->Add(maxLEDs->box, MENUITEMFLAGS);

  return boolOptions;
}
wxBoxSizer* GeneralPage::numOptions(wxStaticBoxSizer* parent) {
  wxBoxSizer* numOptions = new wxBoxSizer(wxVERTICAL);

  buttons = Misc::createNumEntry(parent, "Number of Buttons", wxID_ANY, 1, 3, 2);
  volume = Misc::createNumEntry(parent, "Max Volume", wxID_ANY, 0, 3500, 2000);
  volume->num->SetIncrement(50);
  clash = Misc::createNumEntryDouble(parent, "Clash Threshold", wxID_ANY, 0.1, 5, 3);
  pliTime = Misc::createNumEntry(parent, "PLI Timeout", wxID_ANY, 1, 60, 2);
  idleTime = Misc::createNumEntry(parent, "Idle Timeout", wxID_ANY, 1, 60, 10);
  motionTime = Misc::createNumEntry(parent, "Motion Timeout", wxID_ANY, 1, 60, 15);

  numOptions->Add(buttons->box, FIRSTITEMFLAGS);
  numOptions->Add(volume->box, MENUITEMFLAGS);
  numOptions->Add(clash->box, MENUITEMFLAGS);
  numOptions->Add(pliTime->box, MENUITEMFLAGS);
  numOptions->Add(idleTime->box, MENUITEMFLAGS);
  numOptions->Add(motionTime->box, MENUITEMFLAGS);

  return numOptions;
}
