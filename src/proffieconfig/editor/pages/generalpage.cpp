#include "generalpage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/statbox.h>

GeneralPage::GeneralPage(EditorWindow *parent) : 
    wxPanel(parent), 
    Notifier(this, parent->getOpenConfig().settings.notifyData),
    mParent(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    sizer->Add(setupSection(), wxSizerFlags(0).Expand());
    sizer->AddSpacer(10);
    sizer->Add(optionSection(), wxSizerFlags(1).Expand());

    mCustomOptDlg = new CustomOptionsDlg(mParent);

    bindEvents();
    initializeNotifier();
    SetSizerAndFit(sizer);
}

void GeneralPage::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
        if (mCustomOptDlg->IsShown()) mCustomOptDlg->Raise();
        else mCustomOptDlg->Show();
    }, ID_CustomOptions);
}

void GeneralPage::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    auto& settings{mParent->getOpenConfig().settings};

    if (rebound or id == settings.ID_OS_VERSION) {
        auto *osVersionButton{FindWindow(ID_OSVersion)};
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

wxSizer *GeneralPage::setupSection() {
    auto& config{mParent->getOpenConfig()};

    auto *boardSetup{new wxStaticBoxSizer(
        wxHORIZONTAL,
        this,
        _("Setup"))
    };

    auto *osVersion{new wxButton(boardSetup->GetStaticBox(), ID_OSVersion)};

    auto *board{new PCUI::Choice(
        boardSetup->GetStaticBox(),
        config.settings.board
    )};

    auto *massStorage {new PCUI::CheckBox(
        boardSetup->GetStaticBox(),
        config.settings.massStorage,
        0,
        _("Enable Mass Storage")
    )};
    auto *webUSB {new PCUI::CheckBox(
        boardSetup->GetStaticBox(),
        config.settings.webUSB,
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

wxSizer *GeneralPage::optionSection() {
    auto *optionsSizer{new wxStaticBoxSizer(
        wxHORIZONTAL, this, _("Options")
    )};

    optionsSizer->Add(
        leftOptions(optionsSizer->GetStaticBox()),
        wxSizerFlags(20).Expand()
    );
    optionsSizer->AddStretchSpacer(1);
    optionsSizer->Add(
        rightOptions(optionsSizer->GetStaticBox()),
        wxSizerFlags(20).Expand()
    );

    return optionsSizer;
}

wxSizer *GeneralPage::rightOptions(wxWindow* parent) {
    auto& config{mParent->getOpenConfig()};
    auto *rightOptions{new wxBoxSizer(wxVERTICAL)};

    // TODO: Save State, yes?

    auto *volumeSave{new PCUI::CheckBox(
        parent,
        config.settings.saveVolume,
        0,
        _("Save Volume")
    )};
    auto *presetSave{new PCUI::CheckBox(
        parent,
        config.settings.savePreset,
        0,
        _("Save Preset")
    )};
    auto *colorSave{new PCUI::CheckBox(
        parent,
        config.settings.saveColorChange,
        0,
        _("Save Color")
    )};
    auto *enableOLED{new PCUI::CheckBox(
        parent,
        config.settings.enableOLED,
        0,
        _("Enable OLED")
    )};
    auto *disableColor{new PCUI::CheckBox(
        parent,
        config.settings.disableColorChange,
        0,
        _("Disable Color Change")
    )};
    auto *noTalkie{new PCUI::CheckBox(
        parent,
        config.settings.disableTalkie,
        0,
        _("Disable Talkie")
    )};
    auto *noBasicParsers{new PCUI::CheckBox(
        parent,
        config.settings.disableBasicParserStyles,
        0,
        _("Disable Basic Parser Styles")
    )};
    auto *disableDiagnosticCommands{new PCUI::CheckBox(
        parent,
        config.settings.disableDiagnosticCommands,
        0,
        _("Disable Diagnostic Commands")
    )};

    auto *customOptButton{new wxButton(
        parent,
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

wxSizer *GeneralPage::leftOptions(wxWindow *parent) {
    auto& config{mParent->getOpenConfig()};

    auto *leftOptions{new wxBoxSizer(wxVERTICAL)};

    auto *orientation{new PCUI::Choice(
        parent, 
        config.settings.orientation,
        _("Orientation"),
        wxHORIZONTAL
    )};
    auto *buttons{new PCUI::Numeric(
        parent,
        config.settings.numButtons,
        _("Number of Buttons"),
        wxHORIZONTAL
    )};
    auto *volume{new PCUI::Numeric(
        parent,
        config.settings.volume,
        _("Max Volume"),
        wxHORIZONTAL
    )};
    auto *clash{new PCUI::Decimal(
        parent,
        config.settings.clashThreshold,
        _("Clash Threshold (Gs)"),
        wxHORIZONTAL
    )};
    auto *pliTime{new PCUI::Decimal(
        parent,
        config.settings.pliOffTime,
        _("PLI Timeout (seconds)"),
        wxHORIZONTAL
    )};
    auto *idleTime{new PCUI::Decimal(
        parent,
        config.settings.idleOffTime,
        _("Idle Timeout (minutes)"),
        wxHORIZONTAL
    )};
    auto *motionTime{new PCUI::Decimal(
        parent,
        config.settings.motionTimeout,
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
