#include "generalpage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>

#include "../../core/defines.h"

GeneralPage::GeneralPage(EditorWindow *parent) : 
    wxStaticBoxSizer(wxVERTICAL, parent, ""), 
    mParent(parent) {
    Add(boardSection(this), BOXITEMFLAGS);
    Add(optionSection(this), BOXITEMFLAGS);

    mCustomOptDlg = new CustomOptionsDlg(mParent);

    bindEvents();
    createToolTips();
}

void GeneralPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
        if (mCustomOptDlg->IsShown()) mCustomOptDlg->Raise();
        else mCustomOptDlg->Show();
    }, ID_CustomOptions);
}

// void GeneralPage::createToolTips() const {
//     TIP(board, _("The hardware revision of the physical proffieboard."));
//     TIP(massStorage, _("Enable to access the contents of your proffieboard's SD card via the USB connection."));
//     TIP(webUSB, _("Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info."));
// 
//     TIP(orientation, _("The orientation of the Proffieboard in the saber."));
//     TIP(buttons, _("Physical buttons on the saber.\nNot all prop files support all possible numbers of buttons, and controls may change depending on how many buttons are specified."));
//     TIP(volume, _("Maximum volume level.\nDo not increase unless you know what you are doing, as this can damage your speaker."));
//     TIP(clash, _("Impact required to trigger a clash effect.\nMeasured in Gs."));
//     TIP(pliTime, _("Time (in minutes) since last activity before PLI goes to sleep."));
//     TIP(idleTime, _("Time (in minutes) since last activity before accent LEDs go to sleep."));
//     TIP(motionTime, _("Time (in minutes) since last activity before gesture controls are disabled."));
//     TIP(maxLEDs, _("Maximum number of LEDs in a WS281X blade.\nThis value should not be changed unless you know what you are doing.\nConfigure the length of your blade in the \"Blade Arrays\" page."));
// 
//     TIP(volumeSave, _("Save the volume level between board restarts."));
//     TIP(presetSave, _("Save the currently-selected preset between board restarts."));
//     TIP(colorSave, _("Save color edits to presets."));
// 
//     TIP(enableOLED, _("Enable if you have an OLED/SSD1306 display connected."));
//     TIP(disableColor, _("Disable color change controls."));
//     TIP(noTalkie, _("Use beeps instead of spoken messages for errors, which saves some memory.\nSee the POD page \"What is it beeping?\"."));
//     TIP(noBasicParsers, _("Disable basic styles in the ProffieOS Workbench to save memory."));
//     TIP(disableDiagnosticCommands, _("Disable diagnostic commands in the Serial Monitor to save memory."));
// }

wxStaticBoxSizer* GeneralPage::boardSection(wxStaticBoxSizer* parent) {
    auto config{mParent->getOpenConfig()};

    auto *boardSetup{new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), _("Board Setup"))};

    auto *board{new PCUI::Choice(
        boardSetup->GetStaticBox(),
        config->settings.board
    )};
    auto *massStorage {new PCUI::CheckBox(
        boardSetup->GetStaticBox(),
        config->settings.massStorage,
        0,
        _("Enable Mass Storage")
    )};
    auto *webUSB {new PCUI::CheckBox(
        boardSetup->GetStaticBox(),
        config->settings.webUSB,
        0,
        _("Enable WebUSB")
    )};

    boardSetup->Add(
        board,
        wxSizerFlags(0).Border(wxALL, 10).Center()
    );
    boardSetup->Add(
        massStorage,
        wxSizerFlags(0).Border(wxALL, 10).Center()
    );
    boardSetup->Add(
        webUSB,
        wxSizerFlags(0).Border(wxALL, 10).Center()
    );

    return boardSetup;
}
wxStaticBoxSizer* GeneralPage::optionSection(wxStaticBoxSizer* parent) {
  auto *options{new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), _("Options"))};

  options->Add(leftOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(rightOptions(options), wxSizerFlags(0).Border(wxALL, 5).Expand());

  return options;
}

wxBoxSizer* GeneralPage::rightOptions(wxStaticBoxSizer* parent) {
    auto config{mParent->getOpenConfig()};
    auto *rightOptions{new wxBoxSizer(wxVERTICAL)};

    // TODO: Save State, yes?

    auto *volumeSave{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.saveVolume,
        0,
        _("Save Volume")
    )};
    auto *presetSave{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.savePreset,
        0,
        _("Save Preset")
    )};
    auto *colorSave{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.saveColorChange,
        0,
        _("Save Color")
    )};
    auto *enableOLED{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.enableOLED,
        0,
        _("Enable OLED")
    )};
    auto *disableColor{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.disableColorChange,
        0,
        _("Disable Color Change")
    )};
    auto *noTalkie{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.disableTalkie,
        0,
        _("Disable Talkie")
    )};
    auto *noBasicParsers{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.disableBasicParserStyles,
        0,
        _("Disable Basic Parser Styles")
    )};
    auto *disableDiagnosticCommands{new PCUI::CheckBox(
        parent->GetStaticBox(),
        config->settings.disableDiagnosticCommands,
        0,
        _("Disable Diagnostic Commands")
    )};

    auto *customOptButton{new wxButton(
        parent->GetStaticBox(),
        ID_CustomOptions,
        _("Custom Options...")
    )};

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
    auto config{mParent->getOpenConfig()};

    auto *leftOptions{new wxBoxSizer(wxVERTICAL)};

    auto *orientation{new PCUI::Choice(
        parent->GetStaticBox(), 
        config->settings.orientation,
        _("Orientation"),
        wxHORIZONTAL
    )};
    auto *buttons{new PCUI::Numeric(
        parent->GetStaticBox(),
        config->settings.numButtons,
        wxSP_ARROW_KEYS,
        _("Number of Buttons"),
        wxHORIZONTAL
    )};
    auto *volume{new PCUI::Numeric(
        parent->GetStaticBox(),
        config->settings.volume,
        wxSP_ARROW_KEYS,
        _("Max Volume"),
        wxHORIZONTAL
    )};
    auto *clash{new PCUI::Decimal(
        parent->GetStaticBox(),
        config->settings.clashThreshold,
        wxSP_ARROW_KEYS,
        _("Clash Threshold (Gs)"),
        wxHORIZONTAL
    )};
    auto *pliTime{new PCUI::Decimal(
        parent->GetStaticBox(),
        config->settings.pliOffTime,
        wxSP_ARROW_KEYS,
        _("PLI Timeout (minutes)"),
        wxHORIZONTAL
    )};
    auto *idleTime{new PCUI::Decimal(
        parent->GetStaticBox(),
        config->settings.idleOffTime,
        wxSP_ARROW_KEYS,
        _("Idle Timeout (minutes)"),
        wxHORIZONTAL
    )};
    auto *motionTime{new PCUI::Decimal(
        parent->GetStaticBox(),
        config->settings.motionOffTime,
        wxSP_ARROW_KEYS,
        _("Motion Timeout (minutes)"),
        wxHORIZONTAL
    )};

    leftOptions->Add(orientation, FIRSTITEMFLAGS);
    leftOptions->Add(buttons, MENUITEMFLAGS);
    leftOptions->Add(volume, MENUITEMFLAGS);
    leftOptions->Add(clash, MENUITEMFLAGS);
    leftOptions->Add(pliTime, MENUITEMFLAGS);
    leftOptions->Add(idleTime, MENUITEMFLAGS);
    leftOptions->Add(motionTime, MENUITEMFLAGS);

    return leftOptions;
}
