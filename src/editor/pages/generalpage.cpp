// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/pages/generalpage.h"

#include "core/defines.h"
#include "core/utilities/misc.h"
#include "core/config/configuration.h"

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/tooltip.h>

GeneralPage::GeneralPage(EditorWindow* _parent) : wxStaticBoxSizer(wxVERTICAL, _parent, ""), parent(_parent) {
    Add(boardSection(this), BOXITEMFLAGS);
    Add(optionSection(this), BOXITEMFLAGS);

    customOptDlg = new CustomOptionsDlg(parent);

    bindEvents();
    createToolTips();
}

void GeneralPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent){
        if (customOptDlg->IsShown()) customOptDlg->Raise();
        else customOptDlg->Show();
    }, ID_CustomOptions);
}

void GeneralPage::createToolTips() {
    TIP(board, "The version of proffieboard.");
    TIP(massStorage, "Enable to access the contents of your proffieboard's SD card via the USB connection.");
    TIP(webUSB, "Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info.");

    TIP(orientation, "The orientation of the Proffieboard in the saber.");
    TIP(buttons, "Number of buttons your saber has.\nPlease note not all prop files support all possible numbers of buttons, and controls may change depending on how many buttons are specified.");
    TIP(volume, "Maximum volume level.\n1500 is a good starting value for most speakers, and it is not recommended to go past 2000 unless you know what you are doing, as this can damage your speaker.");
    TIP(clash, "Force required to trigger a clash effect.\nMeasured in Gs.");
    TIP(pliTime, "Time (in minutes) since last activity before PLI goes to sleep.");
    TIP(idleTime, "Time (in minutes) since last activity before accent LEDs go to sleep.");
    TIP(motionTime, "Time (in minutes) since last activity before gesture controls are disabled.");
    TIP(maxLEDs, "Maximum number of LEDs in a WS281X blade.\nThis value should not be changed unless you know what you are doing.\nConfigure the length of your blade in the \"Blade Arrays\" page.");

    TIP(options[SAVE_VOLUME], "Save the volume level between board restarts.");
    TIP(options[SAVE_PRESET], "Save the currently-selected preset between board restarts.");
    TIP(options[SAVE_COLOR], "Save color edits to presets.");

    TIP(options[ENABLE_OLED], "Enable if you have an OLED/SSD1306 display connected.");
    TIP(options[DISABLE_COLOR_CHANGE], "Disable color change controls.");
    TIP(options[DISABLE_TALKIE], "Use beeps for errors instead of spoken errors and can be used to save some memory.\nSee the POD page \"What is it beeping?\".");
    TIP(options[DISABLE_BASIC_PARSER_STYLES], "Disable basic styles for use in the ProffieOS Workbench.\nThis can be used to save memory.");
    TIP(options[DISABLE_DIAGNOSTIC_COMMANDS], "Disable diagnostic commands in the Serial Monitor.\nThis can be used to save memory.");

    TIP(options[DYNAMIC_BLADE_LENGTH], "Allow dynamically changing blade length.")
    TIP(options[DYNAMIC_BLADE_DIMMING], "Allow dynamically changing blade dimming.")
    TIP(options[DYNAMIC_CLASH_THRESHOLD], "Allow dynamically changing clash threshold.")
    TIP(options[SAVE_CLASH_THRESHOLD], "Save clash threshold edits.")
    TIP(options[SAVE_BLADE_DIMMING], "Save blade dimming edits.")
}

wxStaticBoxSizer* GeneralPage::boardSection(wxStaticBoxSizer* parent) {
    wxStaticBoxSizer* boardSetup = new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), "Board Setup");

    board = new pcChoice(boardSetup->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Configuration::Proffieboard), 0);
    massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
    webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

    boardSetup->Add(board, wxSizerFlags(0).Border(wxALL, 10).Center());
    boardSetup->Add(massStorage, wxSizerFlags(0).Border(wxALL, 10).Center());
    boardSetup->Add(webUSB, wxSizerFlags(0).Border(wxALL, 10).Center());

    return boardSetup;
}
wxStaticBoxSizer* GeneralPage::optionSection(wxStaticBoxSizer* parent) {
    wxStaticBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), "Options");

    options->Add(leftOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());
    options->Add(rightOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());

    return options;
}
wxBoxSizer* GeneralPage::rightOptions(wxStaticBoxSizer* parent) {
    wxBoxSizer* rightOptions = new wxBoxSizer(wxVERTICAL);

    options[SAVE_VOLUME] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Volume");
    options[SAVE_PRESET] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Preset");
    options[SAVE_COLOR] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Color");

    options[DYNAMIC_BLADE_LENGTH] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Dynamic Blade Length");
    options[DYNAMIC_BLADE_DIMMING] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Dynamic Blade Dimming");
    options[DYNAMIC_CLASH_THRESHOLD] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Dynamic Clash Threshold");
    options[SAVE_CLASH_THRESHOLD] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Clash Threshold");
    options[SAVE_BLADE_DIMMING] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Save Blade Dimming");

    options[ENABLE_OLED] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Enable OLED");

    options[DISABLE_COLOR_CHANGE] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Color Change");
    options[DISABLE_TALKIE] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Talkie");
    options[DISABLE_BASIC_PARSER_STYLES] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Basic Parser Styles");
    options[DISABLE_DIAGNOSTIC_COMMANDS] = new wxCheckBox(parent->GetStaticBox(), wxID_ANY, "Disable Diagnostic Commands");

    customOptButton = new wxButton(parent->GetStaticBox(), ID_CustomOptions, "Custom Options...");

    rightOptions->Add(options[ENABLE_OLED], MENUITEMFLAGS);
    rightOptions->Add(options[SAVE_VOLUME], FIRSTITEMFLAGS);
    rightOptions->Add(options[SAVE_PRESET], MENUITEMFLAGS);
    rightOptions->Add(options[SAVE_COLOR], MENUITEMFLAGS);
    rightOptions->Add(new wxStaticLine(), MENUITEMFLAGS.Expand());
    rightOptions->Add(options[DISABLE_COLOR_CHANGE], MENUITEMFLAGS);
    rightOptions->Add(options[DISABLE_TALKIE], MENUITEMFLAGS);
    rightOptions->Add(options[DISABLE_BASIC_PARSER_STYLES], MENUITEMFLAGS);
    rightOptions->Add(options[DISABLE_DIAGNOSTIC_COMMANDS], MENUITEMFLAGS);
    rightOptions->Add(new wxStaticLine(), MENUITEMFLAGS.Expand());
    rightOptions->Add(options[DYNAMIC_BLADE_LENGTH], MENUITEMFLAGS);
    rightOptions->Add(options[DYNAMIC_BLADE_DIMMING], MENUITEMFLAGS);
    rightOptions->Add(options[DYNAMIC_CLASH_THRESHOLD], MENUITEMFLAGS);
    rightOptions->Add(options[SAVE_BLADE_DIMMING], MENUITEMFLAGS);
    rightOptions->Add(options[SAVE_CLASH_THRESHOLD], MENUITEMFLAGS);

    rightOptions->Add(customOptButton, wxSizerFlags(1).Border(wxALL, 10).Expand());

    return rightOptions;
}
wxBoxSizer* GeneralPage::leftOptions(wxStaticBoxSizer* parent) {
    wxBoxSizer* leftOptions = new wxBoxSizer(wxVERTICAL);

    orientation = new pcChoice(parent->GetStaticBox(), wxID_ANY, "Orientation", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Configuration::Orientation), 0, wxHORIZONTAL);
    buttons = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Number of Buttons", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3, 2, wxHORIZONTAL);
    volume = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Max Volume", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 5000, 1500, wxHORIZONTAL);
    volume->entry()->SetIncrement(50);
    clash = new pcSpinCtrlDouble(parent->GetStaticBox(), wxID_ANY, "Clash Threshold (Gs)", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.1, 5, 3, wxHORIZONTAL);
    pliTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "PLI Timeout (minutes)", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 2, wxHORIZONTAL);
    idleTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Idle Timeout (minutes)", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 10, wxHORIZONTAL);
    motionTime = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "Motion Timeout (minutes)", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 15, wxHORIZONTAL);
    maxLEDs = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "WS281X Max LEDs", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1024, 144, wxHORIZONTAL);

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
