#include "generalpage.h"
#include "ui/static_box.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/generalpage.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/statline.h>

GeneralPage::GeneralPage(EditorWindow *parent) : 
    wxPanel(parent), 
    NotifyReceiver(this, parent->getOpenConfig().settings.notifyData),
    mParent(parent) {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *sizer0{new wxBoxSizer(wxHORIZONTAL)};
    sizer0->Add(installationSection(), wxSizerFlags().Expand());
    sizer0->AddSpacer(10);
    sizer0->Add(audioSection(), wxSizerFlags().Expand());

    auto *sizer1{new wxBoxSizer(wxVERTICAL)};
    sizer1->Add(setupSection(), wxSizerFlags().Expand());
    sizer1->AddSpacer(10);
    sizer1->Add(sizer0, wxSizerFlags().Expand());
    sizer1->AddSpacer(10);
    sizer1->Add(tweaksSection(), wxSizerFlags().Expand());

    auto *sizer2{new wxBoxSizer(wxVERTICAL)};
    sizer2->Add(editingSection(), wxSizerFlags().Expand());
    sizer2->AddSpacer(10);
    sizer2->Add(miscSection(), wxSizerFlags(1).Expand());

    sizer->Add(sizer1, wxSizerFlags().Expand());
    sizer->AddSpacer(20);
    sizer->Add(sizer2, wxSizerFlags().Expand());

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
}

wxWindow *GeneralPage::setupSection() {
    auto& config{mParent->getOpenConfig()};

    auto *boardSetup{new PCUI::StaticBox(
        wxHORIZONTAL,
        this,
        _("Setup"))
    };

    auto *osVersion{new PCUI::Choice(
        boardSetup,
        config.settings.osVersion
    )};

    auto *board{new PCUI::Choice(
        boardSetup,
        config.settings.board
    )};
    board->SetToolTip(_("The hardware revision of the physical proffieboard.")); 

    auto *massStorage {new PCUI::CheckBox(
        boardSetup,
        config.settings.massStorage,
        0,
        _("Enable Mass Storage")
    )};
    massStorage->SetToolTip(_("Enable to access the contents of your proffieboard's SD card via the USB connection."));

    auto *webUSB {new PCUI::CheckBox(
        boardSetup,
        config.settings.webUSB,
        0,
        _("Enable WebUSB")
    )};
    webUSB->SetToolTip(_("Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info."));

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

wxWindow *GeneralPage::miscSection() {
    auto& config{mParent->getOpenConfig()};
    auto *sizer{new PCUI::StaticBox(wxVERTICAL, this, _("Misc"))};
    auto *parent{sizer};

    auto *pliTime{new PCUI::Decimal(
        parent,
        config.settings.pliOffTime,
        _("PLI Timeout (seconds)"),
        wxVERTICAL
    )};
    pliTime->SetToolTip(_("Time (in minutes) since last activity before PLI goes to sleep."));

    auto *idleTime{new PCUI::Decimal(
        parent,
        config.settings.idleOffTime,
        _("Idle Timeout (minutes)"),
        wxVERTICAL
    )};
    idleTime->SetToolTip(_("Time (in minutes) since last activity before accent LEDs go to sleep."));

    auto *motionTime{new PCUI::Decimal(
        parent,
        config.settings.motionTimeout,
        _("Motion Timeout (minutes)"),
        wxVERTICAL
    )};
    motionTime->SetToolTip(_("Time (in minutes) since last activity before gesture controls are disabled."));
    
    sizer->Add(pliTime, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(idleTime, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(motionTime, wxSizerFlags().Expand());

    return sizer;
}

wxWindow *GeneralPage::installationSection() {
    auto& config{mParent->getOpenConfig()};
    auto *sizer{new PCUI::StaticBox(wxVERTICAL, this, _("Installation"))};
    auto *parent{sizer};

    auto *clash{new PCUI::Decimal(
        parent,
        config.settings.clashThreshold,
        _("Clash Threshold (Gs)"),
        wxHORIZONTAL
    )};
    clash->SetToolTip(_("Impact required to trigger a clash effect.\nMeasured in Gs."));

    auto *line1{new wxStaticLine(parent)};

    auto *orientation{new PCUI::Choice(
        parent, 
        config.settings.orientation,
        _("Orientation"),
        wxHORIZONTAL
    )};
    orientation->SetToolTip(_("The orientation of the Proffieboard in the saber."));

    auto *rotationSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *rotationX{new PCUI::Numeric(
        parent,
        config.settings.orientationRotation.x,
        _("X"),
        wxHORIZONTAL
    )};
    auto *rotationY{new PCUI::Numeric(
        parent,
        config.settings.orientationRotation.y,
        _("Y"),
        wxHORIZONTAL
    )};
    auto *rotationZ{new PCUI::Numeric(
        parent,
        config.settings.orientationRotation.z,
        _("Z"),
        wxHORIZONTAL
    )};
    rotationSizer->Add(rotationX, wxSizerFlags(1));
    rotationSizer->AddSpacer(5);
    rotationSizer->Add(rotationY, wxSizerFlags(1));
    rotationSizer->AddSpacer(5);
    rotationSizer->Add(rotationZ, wxSizerFlags(1));

    auto *line2{new wxStaticLine(parent)};

    auto *buttons{new PCUI::Numeric(
        parent,
        config.settings.numButtons,
        _("Number of Buttons"),
        wxHORIZONTAL
    )};
    buttons->SetToolTip(_("Physical buttons on the saber.\nNot all prop files support all possible numbers of buttons, and controls may change depending on how many buttons are specified."));

    auto *line3{new wxStaticLine(parent)};

    auto *enableOLED{new PCUI::CheckBox(
        parent,
        config.settings.enableOLED,
        0,
        _("Enable OLED")
    )};
    enableOLED->SetToolTip(_("Enable if you have an OLED/SSD1306 display connected."));

    sizer->Add(clash, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line1, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(orientation, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(rotationSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line2, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(buttons, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line3, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(enableOLED, wxSizerFlags().Expand());

    return sizer;
}

wxWindow *GeneralPage::tweaksSection() {
    auto& config{mParent->getOpenConfig()};
    auto *sizer{new PCUI::StaticBox(wxHORIZONTAL, this, _("Tweaks"))};
    auto *parent{sizer};

    auto *noRepeatRandom{new PCUI::CheckBox(
        parent,
        config.settings.noRepeatRandom,
        0,
        _("No Repeat Random")
    )};
    noRepeatRandom->SetToolTip(_("Do not play the same audio file twice consecutively when randomly selecting."));

    auto *killOldPlayers{new PCUI::CheckBox(
        parent,
        config.settings.killOldPlayers,
        0,
        _("Kill Old Players")
    )};
    killOldPlayers->SetToolTip(_("Stop playing old sounds to ensure new sounds always play"));

    auto *disableColor{new PCUI::CheckBox(
        parent,
        config.settings.disableColorChange,
        0,
        _("Disable Color Change")
    )};
    disableColor->SetToolTip(_("Disable color change controls."));

    auto *noTalkie{new PCUI::CheckBox(
        parent,
        config.settings.disableTalkie,
        0,
        _("Disable Talkie")
    )};
    noTalkie->SetToolTip(_("Use beeps instead of spoken messages for errors, which saves some memory.\nSee the POD page \"What is it beeping?\"."));

    auto *femaleTalkie{new PCUI::CheckBox(
        parent,
        config.settings.femaleTalkie,
        0,
        _("Female Talkie")
    )};
    femaleTalkie->SetToolTip(_("Use Female Talkie Voice"));

    auto *noBasicParsers{new PCUI::CheckBox(
        parent,
        config.settings.disableBasicParserStyles,
        0,
        _("Disable Basic Parser Styles")
    )};
    noBasicParsers->SetToolTip(_("Disable basic styles in the ProffieOS Workbench to save memory."));

    auto *disableDiagnosticCommands{new PCUI::CheckBox(
        parent,
        config.settings.disableDiagnosticCommands,
        0,
        _("Disable Diagnostic Commands")
    )};
    disableDiagnosticCommands->SetToolTip(_("Disable diagnostic commands in the Serial Monitor to save memory."));

    auto *customOptButton{new wxButton(
        parent,
        ID_CustomOptions,
        _("Custom Options...")
    )};

    auto *sizer1{new wxBoxSizer(wxVERTICAL)};
    sizer1->Add(noRepeatRandom, wxSizerFlags().Expand());
    sizer1->AddSpacer(5);
    sizer1->Add(killOldPlayers, wxSizerFlags().Expand());
    sizer1->AddSpacer(5);
    sizer1->Add(noTalkie, wxSizerFlags().Expand());
    sizer1->AddSpacer(5);
    sizer1->Add(femaleTalkie, wxSizerFlags().Expand());

    auto *sizer2{new wxBoxSizer(wxVERTICAL)};
    sizer2->Add(disableColor, wxSizerFlags().Expand());
    sizer2->AddSpacer(5);
    sizer2->Add(noBasicParsers, wxSizerFlags().Expand());
    sizer2->AddSpacer(5);
    sizer2->Add(disableDiagnosticCommands, wxSizerFlags().Expand());
    sizer2->AddSpacer(5);
    sizer2->Add(customOptButton, wxSizerFlags().Expand());

    sizer->Add(sizer1, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(sizer2, wxSizerFlags().Expand());

    return sizer;
}

wxWindow *GeneralPage::editingSection() {
    auto& config{mParent->getOpenConfig()};
    auto *sizer{new PCUI::StaticBox(wxVERTICAL, this, _("Editing"))};
    auto *parent{sizer};

    auto *saveState{new PCUI::CheckBox(
        parent,
        config.settings.saveState,
        0,
        _("Save State")
    )};

    auto *allEditOpts{new PCUI::CheckBox(
        parent,
        config.settings.enableAllEditOptions,
        0,
        _("Enable All Edit Options")
    )};

    auto *line1{new wxStaticLine(parent)};

    auto *saveVolume{new PCUI::CheckBox(
        parent,
        config.settings.saveVolume,
        0,
        _("Save Volume")
    )};
    saveVolume->SetToolTip(_("Save the volume level between board restarts."));

    auto *savePreset{new PCUI::CheckBox(
        parent,
        config.settings.savePreset,
        0,
        _("Save Preset")
    )};
    savePreset->SetToolTip(_("Save the currently-selected preset between board restarts."));

    auto *saveColor{new PCUI::CheckBox(
        parent,
        config.settings.saveColorChange,
        0,
        _("Save Color")
    )};
    saveColor->SetToolTip(_("Save color edits to presets."));

    auto *saveDimming{new PCUI::CheckBox(
        parent,
        config.settings.saveBladeDimming,
        0,
        _("Save Blade Dimming")
    )};
    auto *saveClashThreshold{new PCUI::CheckBox(
        parent,
        config.settings.saveClashThreshold,
        0,
        _("Save Clash Threshold")
    )};

    auto *line2{new wxStaticLine(parent)};

    auto *dynamicDimming{new PCUI::CheckBox(
        parent,
        config.settings.dynamicBladeDimming,
        0,
        _("Dynamic Blade Dimming")
    )};

    auto *dynamicClashThreshold{new PCUI::CheckBox(
        parent,
        config.settings.dynamicClashThreshold,
        0,
        _("Dynamic Clash Threshold")
    )};

    auto *dynamicLength{new PCUI::CheckBox(
        parent,
        config.settings.dynamicBladeLength,
        0,
        _("Dynamic Blade Length")
    )};

    sizer->Add(saveState, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(allEditOpts, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line1, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(saveVolume, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(savePreset, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(saveColor, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(saveDimming, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(saveClashThreshold, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(line2, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(dynamicDimming, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(dynamicClashThreshold, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(dynamicLength, wxSizerFlags().Expand());

    return sizer;
}

wxWindow *GeneralPage::audioSection() {
    auto& config{mParent->getOpenConfig()};
    auto *sizer{new PCUI::StaticBox(wxVERTICAL, this, _("Audio"))};
    auto *parent{sizer};

    auto *volume{new PCUI::Numeric(
        parent,
        config.settings.volume,
        _("Max Volume"),
        wxHORIZONTAL
    )};
    volume->SetToolTip(_("Maximum volume level.\nDo not increase unless you know what you are doing, as this can damage your speaker."));

    auto *bootVolumeSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *enableBootVolume{new PCUI::CheckBox(
        parent,
        config.settings.enableBootVolume
    )};
    auto *bootVolume{new PCUI::Numeric(
        parent,
        config.settings.bootVolume,
        _("Boot Volume"),
        wxHORIZONTAL
    )};
    bootVolume->SetToolTip(_("Saber volume when saber turns on. Volume can be changed afterwards."));
    bootVolumeSizer->Add(enableBootVolume, wxSizerFlags().Center());
    bootVolumeSizer->AddSpacer(5);
    bootVolumeSizer->Add(bootVolume, wxSizerFlags(1));

    auto *line1{new wxStaticLine(parent)};

    auto *enableFilter{new PCUI::CheckBox(
        parent,
        config.settings.enableFiltering,
        0,
        _("Enable Filtering")
    )};
    auto *filterCutoff{new PCUI::Numeric(
        parent,
        config.settings.filterCutoff,
        _("Cutoff"),
        wxHORIZONTAL
    )};
    auto *filterOrder{new PCUI::Numeric(
        parent,
        config.settings.filterOrder,
        _("Order"),
        wxHORIZONTAL
    )};

    auto *line2{new wxStaticLine(parent)};

    auto *clashSuppression{new PCUI::Numeric(
        parent,
        config.settings.audioClashSuppressionLevel,
        _("Clash Suppression"),
        wxHORIZONTAL
    )};
    auto *dontUseGyroForClash{new PCUI::CheckBox(
        parent,
        config.settings.dontUseGyroForClash,
        0,
        _("Do Not Use Gyro For Clash")
    )};

    sizer->Add(volume, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(bootVolumeSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line1, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(enableFilter, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(filterCutoff, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(filterOrder, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(line2, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(clashSuppression, wxSizerFlags().Expand());
    sizer->AddSpacer(5);
    sizer->Add(dontUseGyroForClash, wxSizerFlags().Expand());

    return sizer;
}

