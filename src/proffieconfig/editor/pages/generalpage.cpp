#include "generalpage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/statbox.h>

GeneralPage::GeneralPage(EditorWindow *parent) : 
    wxStaticBoxSizer(wxVERTICAL, parent, ""), 
    Notifier(GetStaticBox(), parent->getOpenConfig()->settings.notifyData),
    mParent(parent) {

    Add(setupSection(this), wxSizerFlags(0).Expand());
    AddSpacer(10);
    Add(optionSection(this), wxSizerFlags(1).Expand());

    mCustomOptDlg = new CustomOptionsDlg(mParent);

    bindEvents();
    initializeNotifier();
}

void GeneralPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
        if (mCustomOptDlg->IsShown()) mCustomOptDlg->Raise();
        else mCustomOptDlg->Show();
    }, ID_CustomOptions);
}

void GeneralPage::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    auto& settings{mParent->getOpenConfig()->settings};

    if (rebound or id == settings.ID_OS_VERSION) {
        auto *osVersionButton{GetStaticBox()->FindWindow(ID_OSVersion)};
        if (settings.osVersion != -1) {
            osVersionButton->SetLabel("OS " + static_cast<string>(settings.osVersion));
        } else {
            osVersionButton->SetLabel("No OS Selected");
        }
    }
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

wxStaticBoxSizer* GeneralPage::setupSection(wxStaticBoxSizer* parent) {
    auto config{mParent->getOpenConfig()};

    auto *boardSetup{new wxStaticBoxSizer(
        wxHORIZONTAL,
        parent->GetStaticBox(),
        _("Setup"))
    };

    auto *osVersion{new wxButton(boardSetup->GetStaticBox(), ID_OSVersion)};

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

    auto sizerFlags{wxSizerFlags().Expand()};
    auto *boardAndVersionSizer{new wxBoxSizer(wxVERTICAL)};
    boardAndVersionSizer->Add(board, sizerFlags);
    boardAndVersionSizer->AddSpacer(5);
    boardAndVersionSizer->Add(osVersion, sizerFlags);

    boardSetup->Add(boardAndVersionSizer, wxSizerFlags().Center());
    boardSetup->AddSpacer(20);
    boardSetup->Add(massStorage, wxSizerFlags().Center());
    boardSetup->AddSpacer(10);
    boardSetup->Add(webUSB, wxSizerFlags().Center());

    return boardSetup;
}

wxStaticBoxSizer* GeneralPage::optionSection(wxStaticBoxSizer* parent) {
    auto *options{new wxStaticBoxSizer(wxHORIZONTAL, parent->GetStaticBox(), _("Options"))};

    options->Add(
        leftOptions(options),
        wxSizerFlags(20).Expand()
    );
    options->AddStretchSpacer(1);
    options->Add(
        rightOptions(options),
        wxSizerFlags(20).Expand()
    );

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

    auto sizerFlags{wxSizerFlags(1).Expand()};
    rightOptions->Add(volumeSave, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(presetSave, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(colorSave, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(enableOLED, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(disableColor, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(noTalkie, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(noBasicParsers, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(disableDiagnosticCommands, sizerFlags);
    rightOptions->AddSpacer(5);
    rightOptions->Add(customOptButton, sizerFlags);

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

    auto sizerFlags{wxSizerFlags(1).Expand()};
    leftOptions->Add(orientation, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(buttons, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(volume, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(clash, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(pliTime, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(idleTime, sizerFlags);
    leftOptions->AddSpacer(5);
    leftOptions->Add(motionTime, sizerFlags);

    return leftOptions;
}
