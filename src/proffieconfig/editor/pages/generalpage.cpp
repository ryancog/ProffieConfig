#include "generalpage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/defines.h"
#include "../../core/utilities/misc.h"
#include "../../core/config/configuration.h"
#include "ui/controls.h"

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

GeneralPage::GeneralPage(EditorWindow* _parent) : wxStaticBoxSizer(wxVERTICAL, _parent, ""), mParent(_parent) {
  Add(boardSection(this), BOXITEMFLAGS);
  Add(optionSection(this), BOXITEMFLAGS);

  customOptDlg = new CustomOptionsDlg(mParent);

  bindEvents();
  createToolTips();
}

void GeneralPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
      if (customOptDlg->IsShown()) customOptDlg->Raise();
      else customOptDlg->Show();
    }, ID_CustomOptions);
}

void GeneralPage::createToolTips() const {
  TIP(board, _("The hardware revision of the physical proffieboard."));
  TIP(massStorage, _("Enable to access the contents of your proffieboard's SD card via the USB connection."));
  TIP(webUSB, _("Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info."));

  TIP(orientation, _("The orientation of the Proffieboard in the saber."));
  TIP(buttons, _("Physical buttons on the saber.\nNot all prop files support all possible numbers of buttons, and controls may change depending on how many buttons are specified."));
  TIP(volume, _("Maximum volume level.\nDo not increase unless you know what you are doing, as this can damage your speaker."));
  TIP(clash, _("Impact required to trigger a clash effect.\nMeasured in Gs."));
  TIP(pliTime, _("Time (in minutes) since last activity before PLI goes to sleep."));
  TIP(idleTime, _("Time (in minutes) since last activity before accent LEDs go to sleep."));
  TIP(motionTime, _("Time (in minutes) since last activity before gesture controls are disabled."));
  TIP(maxLEDs, _("Maximum number of LEDs in a WS281X blade.\nThis value should not be changed unless you know what you are doing.\nConfigure the length of your blade in the \"Blade Arrays\" page."));

  TIP(volumeSave, _("Save the volume level between board restarts."));
  TIP(presetSave, _("Save the currently-selected preset between board restarts."));
  TIP(colorSave, _("Save color edits to presets."));

  TIP(enableOLED, _("Enable if you have an OLED/SSD1306 display connected."));
  TIP(disableColor, _("Disable color change controls."));
  TIP(noTalkie, _("Use beeps instead of spoken messages for errors, which saves some memory.\nSee the POD page \"What is it beeping?\"."));
  TIP(noBasicParsers, _("Disable basic styles in the ProffieOS Workbench to save memory."));
  TIP(disableDiagnosticCommands, _("Disable diagnostic commands in the Serial Monitor to save memory."));
}

wxStaticBoxSizer* GeneralPage::boardSection(wxStaticBoxSizer* parent) {
  auto *boardSetup{new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), _("Board Setup"))};

  board = new PCUI::Choice(boardSetup->GetStaticBox(), wxID_ANY, Misc::createEntries(Configuration::PROFFIEBOARD));
  massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, _("Enable Mass Storage"));
  webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, _("Enable WebUSB"));

  boardSetup->Add(board, wxSizerFlags(0).Border(wxALL, 10).Center());
  boardSetup->Add(massStorage, wxSizerFlags(0).Border(wxALL, 10).Center());
  boardSetup->Add(webUSB, wxSizerFlags(0).Border(wxALL, 10).Center());

  return boardSetup;
}
wxStaticBoxSizer* GeneralPage::optionSection(wxStaticBoxSizer* parent) {
  auto *options{new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), _("Options"))};

  options->Add(leftOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(rightOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());

  return options;
}

wxBoxSizer* GeneralPage::rightOptions(wxStaticBoxSizer* parent) {
    auto *rightOptions{new wxBoxSizer(wxVERTICAL)};

    volumeSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Save Volume"));
    presetSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Save Preset"));
    colorSave = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Save Color"));
    enableOLED = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Enable OLED"));
    disableColor = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Disable Color Change"));
    noTalkie = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Disable Talkie"));
    noBasicParsers = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Disable Basic Parser Styles"));
    disableDiagnosticCommands = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, _("Disable Diagnostic Commands"));

    customOptButton = new wxButton(parent->GetStaticBox(), ID_CustomOptions, _("Custom Options..."));

    rightOptions->Add(volumeSave, FIRSTITEMFLAGS);
    rightOptions->Add(presetSave, MENUITEMFLAGS);
    rightOptions->Add(colorSave, MENUITEMFLAGS);
    rightOptions->Add(enableOLED, MENUITEMFLAGS);
    rightOptions->Add(disableColor, MENUITEMFLAGS);
    rightOptions->Add(noTalkie, MENUITEMFLAGS);
    rightOptions->Add(noBasicParsers, MENUITEMFLAGS);
    rightOptions->Add(disableDiagnosticCommands, MENUITEMFLAGS);

    rightOptions->Add(customOptButton, wxSizerFlags(1).Border(wxALL, 10).Expand());

    return rightOptions;
}

wxBoxSizer* GeneralPage::leftOptions(wxStaticBoxSizer* parent) {
    auto *leftOptions{new wxBoxSizer(wxVERTICAL)};

    orientation = new PCUI::Choice(parent->GetStaticBox(), wxID_ANY,  Misc::createEntries(Configuration::ORIENTATION), _("Orientation"), wxHORIZONTAL);
    buttons = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 0, 3, 2, 1, wxSP_ARROW_KEYS, _("Number of Buttons"), wxHORIZONTAL);
    volume = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 0, 5000, 1500, 50, wxSP_ARROW_KEYS, _("Max Volume"), wxHORIZONTAL);
    clash = new PCUI::NumericDec(parent->GetStaticBox(), wxID_ANY, 0.1, 5, 3, 0.1, wxSP_ARROW_KEYS, _("Clash Threshold (Gs)"), wxHORIZONTAL);
    pliTime = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 1, 60, 2, 1, wxSP_ARROW_KEYS, _("PLI Timeout (minutes)"), wxHORIZONTAL);
    idleTime = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 1, 60, 10, 1, wxSP_ARROW_KEYS, _("Idle Timeout (minutes)"), wxHORIZONTAL);
    motionTime = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 1, 60, 15, 1, wxSP_ARROW_KEYS, _("Motion Timeout (minutes)"), wxHORIZONTAL);
    maxLEDs = new PCUI::Numeric(parent->GetStaticBox(), wxID_ANY, 0, 1024, 144, 1, wxSP_ARROW_KEYS, _("WS281X Max LEDs"), wxHORIZONTAL);

    leftOptions->Add(orientation, FIRSTITEMFLAGS);
    leftOptions->Add(buttons, MENUITEMFLAGS);
    leftOptions->Add(volume, MENUITEMFLAGS);
    leftOptions->Add(clash, MENUITEMFLAGS);
    leftOptions->Add(pliTime, MENUITEMFLAGS);
    leftOptions->Add(idleTime, MENUITEMFLAGS);
    leftOptions->Add(motionTime, MENUITEMFLAGS);
    leftOptions->Add(maxLEDs, MENUITEMFLAGS);

    return leftOptions;
}
