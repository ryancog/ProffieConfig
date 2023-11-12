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

  settings.board = new wxComboBox(boardSetup->GetStaticBox(), wxID_ANY, "ProffieBoard V2", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}), wxCB_READONLY);
  settings.massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
  settings.webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

  boardSetup->Add(settings.board, FIRSTITEMFLAGS);
  boardSetup->Add(settings.massStorage, MENUITEMFLAGS);
  boardSetup->Add(settings.webUSB, MENUITEMFLAGS);

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

  settings.volumeSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Volume");
  settings.presetSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Preset");
  settings.colorSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Color");
  settings.disableColor = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Color Change");
  settings.noTalkie = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Talkie");
  settings.noBasicParsers = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Basic Parser Styles");
  settings.disableDiagnosticCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Diagnostic Commands");
  settings.enableDeveloperCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Enable Developer Commands");
  settings.maxLEDs = Misc::createNumEntry(parent, "WS281X Max LEDs", wxID_ANY, 0, 1024, 144);

  boolOptions->Add(settings.volumeSave, FIRSTITEMFLAGS);
  boolOptions->Add(settings.presetSave, MENUITEMFLAGS);
  boolOptions->Add(settings.colorSave, MENUITEMFLAGS);
  boolOptions->Add(settings.disableColor, MENUITEMFLAGS);
  boolOptions->Add(settings.noTalkie, MENUITEMFLAGS);
  boolOptions->Add(settings.noBasicParsers, MENUITEMFLAGS);
  boolOptions->Add(settings.disableDiagnosticCommands, MENUITEMFLAGS);
  boolOptions->Add(settings.enableDeveloperCommands, MENUITEMFLAGS);
  boolOptions->Add(settings.maxLEDs->box, MENUITEMFLAGS);

  return boolOptions;
}
wxBoxSizer* GeneralPage::numOptions(wxStaticBoxSizer* parent) {
  wxBoxSizer* numOptions = new wxBoxSizer(wxVERTICAL);

  settings.buttons = Misc::createNumEntry(parent, "Number of Buttons", wxID_ANY, 1, 3, 2);
  settings.volume = Misc::createNumEntry(parent, "Max Volume", wxID_ANY, 0, 3500, 2000);
  settings.volume->num->SetIncrement(50);
  settings.clash = Misc::createNumEntryDouble(parent, "Clash Threshold", wxID_ANY, 0.1, 5, 3);
  settings.pliTime = Misc::createNumEntry(parent, "PLI Timeout", wxID_ANY, 1, 60, 2);
  settings.idleTime = Misc::createNumEntry(parent, "Idle Timeout", wxID_ANY, 1, 60, 10);
  settings.motionTime = Misc::createNumEntry(parent, "Motion Timeout", wxID_ANY, 1, 60, 15);

  numOptions->Add(settings.buttons->box, FIRSTITEMFLAGS);
  numOptions->Add(settings.volume->box, MENUITEMFLAGS);
  numOptions->Add(settings.clash->box, MENUITEMFLAGS);
  numOptions->Add(settings.pliTime->box, MENUITEMFLAGS);
  numOptions->Add(settings.idleTime->box, MENUITEMFLAGS);
  numOptions->Add(settings.motionTime->box, MENUITEMFLAGS);

  return numOptions;
}
