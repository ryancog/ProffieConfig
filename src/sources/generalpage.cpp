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



GeneralPage::GeneralPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "")
{
  generalHoriz = new wxBoxSizer(wxHORIZONTAL);
  boardSetup = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), "Board Setup");
  options = new wxStaticBoxSizer(wxHORIZONTAL, GetStaticBox(), "Options");


  wxBoxSizer *options1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *options2 = new wxBoxSizer(wxVERTICAL);

  createBoardSettings();

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

  Add(generalHoriz, wxSizerFlags(0).Expand());
  Add(options, wxSizerFlags(0).Border(wxALL, 10).Expand());
}


decltype(GeneralPage::settings) GeneralPage::settings;
