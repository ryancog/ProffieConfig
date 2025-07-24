#include "awarenessdlg.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/tooltip.h>
#include <wx/statbox.h>

#include "../../core/defines.h"
#include "../editorwindow.h"

BladeAwarenessDlg::BladeAwarenessDlg(EditorWindow* parent) :
    wxDialog(
        parent,
        wxID_ANY,
        _("Blade Awareness") + " - " + static_cast<string>(parent->getOpenConfig()->name),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    mParent(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *enableSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *enableDetect{new PCUI::CheckBox(
        this,
        parent->getOpenConfig()->settings.bladeID.enable,
        0,
        _("Enable Blade Detect")
    )};
    auto *enableID{new PCUI::CheckBox(
        this,
        parent->getOpenConfig()->settings.bladeDetect,
        0,
        _("Enable Blade ID")
    )};
    enableID->SetToolTip(_("Detect when a specific blade is inserted based on a resistor placed in the blade to give it an identifier."));
    enableDetect->SetToolTip(_("Detect when a blade is inserted into the saber or not."));
    enableSizer->Add(enableDetect);
    enableSizer->Add(enableID);

    auto *topSizer{new wxBoxSizer(wxHORIZONTAL)};
    topSizer->Add(
        createBladeDetect(this),
        wxSizerFlags(2).Border(wxALL, 10).Expand()
    );
    topSizer->Add(
        createIDSetup(this),
        wxSizerFlags(3).Border(wxALL, 10).Expand()
    );

    auto *bottomSizer{new wxBoxSizer(wxHORIZONTAL)};
    bottomSizer->Add(
        createIDPowerSettings(this),
        wxSizerFlags(1).Border(wxALL, 10).Expand()
    );
    bottomSizer->Add(
        createContinuousScanSettings(this),
        wxSizerFlags(1).Border(wxALL, 10).Expand()
    );

    sizer->Add(enableSizer);
    sizer->Add(topSizer, wxSizerFlags(1).Expand());
    sizer->Add(bottomSizer, wxSizerFlags(0).Expand());

    SetSizerAndFit(sizer);
    bindEvents();
    createToolTips();
}

void BladeAwarenessDlg::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        if (event.CanVeto()) {
            Hide();
            event.Veto();
        } else event.Skip();
    });
}

void BladeAwarenessDlg::createToolTips() const {
    // TIP(detectPin, _("The pin which will be bridged to BATT- when blade is inserted.\nCannot be the same as ID Pin."));
    // TIP(IDPin, _("The pin used to detect blade resistance values.\nCannot be the same as Detect Pin."));
    // TIP(mode, _("The mode to be used for Blade ID.\nSee the POD page \"Blade ID\" for more info."));
    // TIP(pullupResistance, _("The value of the pullup resistor placed on the Blade ID line."));
    // TIP(pullupPin, _("The pin number or name of the pin which ID Pin is bridged to for pullup.\n This pin cannot be used for anything else."));

    // TIP(enablePowerForID, _("Enable power during Blade ID.\nThis is required for WS281X blades."));
    // TIP(continuousScans, _("Continuously monitor the Blade ID to detect changes."));
    // TIP(scanIDMillis, _("Scan the Blade ID and update accordingly every input number of millis."));
    // TIP(numIDTimes, _("Number of times to read the Blade ID to average out the result and compensate for inaccurate readings."));

    // TIP(addID, _("Add a blade array which will be enabled when a blade with the specified ID is inserted."));
    // TIP(removeID, _("Remove the selected blade array."));

    // TIP(arrayName, _("The name of the blade array.\nEach name must be unique, but it is for reference only (and thus specific names will not make a difference)."));
    // TIP(resistanceID, _("The ID of the blade associated with the currently-selected blade array.\nThis value can be measured by typing \"id\" into the Serial Monitor."));
}

wxStaticBoxSizer* BladeAwarenessDlg::createIDSetup(wxWindow* parent) {
    auto config{mParent->getOpenConfig()};

    auto *setupSizer{new wxStaticBoxSizer(
        wxVERTICAL,
        parent,
        _("Blade ID Setup")
    )};

    auto *mode{new PCUI::Choice(
        setupSizer->GetStaticBox(),
        config->settings.bladeID.mode,
        _("Blade ID Mode")
    )};
    auto *idPin{new PCUI::ComboBox(
        setupSizer->GetStaticBox(),
        config->settings.bladeID.pin,
        _("Blade ID Pin")
    )};

    auto *pullupResistance{new PCUI::Numeric(
        setupSizer->GetStaticBox(),
        config->settings.bladeID.pullup,
        wxSP_ARROW_KEYS, 
        _("Pullup Resistance")
    )};
    auto *pullupPin{new PCUI::ComboBox(
        setupSizer->GetStaticBox(),
        config->settings.bladeID.bridgePin,
        _("Pullup Pin")
    )};

    setupSizer->Add(mode);
    setupSizer->Add(idPin);
    setupSizer->Add(pullupResistance);
    setupSizer->Add(pullupPin);

    return setupSizer;
}

wxStaticBoxSizer* BladeAwarenessDlg::createIDPowerSettings(wxWindow* parent) {
    auto config{mParent->getOpenConfig()};

    auto *powerForIDSizer{new wxStaticBoxSizer(
        wxVERTICAL,
        parent,
        _("Power for Blade ID")
    )};

    auto *enablePowerForID {new PCUI::CheckBox(
        powerForIDSizer->GetStaticBox(),
        config->settings.bladeID.powerForID,
        0,
        _("Enable Power on ID")
    )};

    auto *powerPins{new PCUI::CheckList(
        powerForIDSizer->GetStaticBox(),
        config->settings.bladeID.powerPins
    )};

    // TODO: Allow powerPin add

    powerForIDSizer->Add(enablePowerForID);
    powerForIDSizer->Add(powerPins);

    return powerForIDSizer;
}

wxStaticBoxSizer* BladeAwarenessDlg::createContinuousScanSettings(wxWindow* parent) {
    auto config{mParent->getOpenConfig()};

    auto *continuousScansSizer{new wxStaticBoxSizer(
        wxVERTICAL,
        parent,
        _("Continuous Scanning")
    )};
    auto *continuousScans{new PCUI::CheckBox(
        continuousScansSizer->GetStaticBox(),
        config->settings.bladeID.continuousScanning,
        0,
        _("Enable Continuous Scanning")
    )};

    auto *numIDTimes{new PCUI::Numeric(
        continuousScansSizer->GetStaticBox(),
        config->settings.bladeID.continuousTimes,
        wxSP_ARROW_KEYS,
        _("Number of Reads to Average")
    )};
    auto *scanIDMillis{new PCUI::Numeric(
        continuousScansSizer->GetStaticBox(),
        config->settings.bladeID.continuousInterval, 
        wxSP_ARROW_KEYS,
        _("Scan Interval (ms)")
    )};
    continuousScansSizer->Add(continuousScans);
    continuousScansSizer->Add(numIDTimes);
    continuousScansSizer->Add(scanIDMillis);

    return continuousScansSizer;
}

wxStaticBoxSizer* BladeAwarenessDlg::createBladeDetect(wxWindow* parent) {
    auto config{mParent->getOpenConfig()};

    auto *bladeDetectSizer{new wxStaticBoxSizer(
        wxVERTICAL,
        parent,
        _("Blade Detect")
    )};

    auto *detectPin{new PCUI::ComboBox(
        bladeDetectSizer->GetStaticBox(),
        config->settings.bladeDetectPin,
        _("Blade Detect Pin")
    )};
    bladeDetectSizer->Add(detectPin, wxSizerFlags(0).Border(wxALL, 5).Expand());

    return bladeDetectSizer;
}

// void BladeAwarenessDlg::update() {
//     if (mLastArraySelection >= 0 && mLastArraySelection < static_cast<int32_t>(bladeArrays.size())) {
//         bladeArrays.at(mLastArraySelection).name = arrayName->entry()->GetValue().ToStdString();
//         bladeArrays.at(mLastArraySelection).value = resistanceID->entry()->GetValue();
//     }
// 
//     mLastArraySelection = arrayList->GetSelection();
//     arrayList->Clear();
// 
//     for (int32_t array = 0; array < static_cast<int32_t>(bladeArrays.size()); array++) {
//         if (arrayList->FindString(bladeArrays.at(array).name) == wxNOT_FOUND) arrayList->Append(bladeArrays.at(array).name);
//         else bladeArrays.erase(bladeArrays.begin() + array);
//     }
//     if (mLastArraySelection < 0 || mLastArraySelection >= static_cast<int32_t>(arrayList->GetCount())) mLastArraySelection = 0;
//     arrayList->SetSelection(mLastArraySelection);
// 
//     detectPin->entry()->Enable(enableDetect->GetValue());
// 
//     arrayList->Enable(enableID->GetValue());
//     mode->entry()->Enable(enableID->GetValue());
//     IDPin->entry()->Enable(enableID->GetValue());
//     pullupPin->entry()->Enable(enableID->GetValue());
//     pullupResistance->entry()->Enable(enableID->GetValue());
//     enablePowerForID->Enable(enableID->GetValue());
//     powerPin1->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     powerPin2->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     powerPin3->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     powerPin4->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     powerPin5->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     powerPin6->Enable(enableID->GetValue() && enablePowerForID->GetValue());
//     continuousScans->Enable(enableID->GetValue());
//     numIDTimes->entry()->Enable(enableID->GetValue() && continuousScans->GetValue());
//     scanIDMillis->entry()->Enable(enableID->GetValue() && continuousScans->GetValue());
//     addID->Enable(enableID->GetValue());
//     removeID->Enable(enableID->GetValue() && mLastArraySelection && !(arrayList->GetStringSelection() == "blade_in" || arrayList->GetStringSelection() == "no_blade"));
// 
//     bool customBladeArraySelected = bladeArrays[mLastArraySelection].name != "no_blade" && bladeArrays[mLastArraySelection].name != "blade_in";
//     arrayName->entry()->Enable(customBladeArraySelected);
//     resistanceID->entry()->Enable(customBladeArraySelected);
//     if (customBladeArraySelected) resistanceID->entry()->SetRange(2000, 100000);
//     else resistanceID->entry()->SetRange(0, 0);
//     arrayName->entry()->ChangeValue(bladeArrays.at(mLastArraySelection).name);
//     resistanceID->entry()->SetValue(bladeArrays.at(mLastArraySelection).value);
// 
//     if (mode->entry()->GetSelection() == BLADE_ID_MODE_SNAPSHOT) {
//         pullupResistance->Show(false);
//         pullupPin->Show(false);
//     } else if (mode->entry()->GetSelection() == BLADE_ID_MODE_BRIDGED) {
//         pullupResistance->Show(false);
//         pullupPin->Show(true);
//     } else if (mode->entry()->GetSelection() == BLADE_ID_MODE_EXTERNAL) {
//         pullupPin->Show(false);
//         pullupResistance->Show(true);
//     }
// 
//     if (mParent->bladesPage && mParent->presetsPage) {
//         mParent->bladesPage->update();
//         mParent->presetsPage->update();
//     }
// 
//     Layout();
//     GetSizer()->Fit(this);
// #   ifdef __WINDOWS__
//     Refresh();
// #   endif
// }

