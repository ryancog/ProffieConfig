#include "awarenessdlg.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/tooltip.h>
#include <wx/statbox.h>

#include "ui/static_box.h"

#include "../editorwindow.h"

BladeAwarenessDlg::BladeAwarenessDlg(EditorWindow* parent) :
    wxDialog(
        parent,
        wxID_ANY,
        _("Blade Awareness") + " - " + static_cast<string>(parent->getOpenConfig().name),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE
    ),
    mParent(parent) {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *topSizer{new wxBoxSizer(wxHORIZONTAL)};
    topSizer->AddSpacer(10);
    topSizer->Add(
        createBladeDetect(this),
        wxSizerFlags(1).Expand()
    );
    topSizer->AddSpacer(10);
    topSizer->Add(
        createIDSetup(this),
        wxSizerFlags(1).Expand()
    );
    topSizer->AddSpacer(10);

    auto *bottomSizer{new wxBoxSizer(wxHORIZONTAL)};
    bottomSizer->AddSpacer(10);
    bottomSizer->Add(
        createIDPowerSettings(this),
        wxSizerFlags(1).Expand()
    );
    bottomSizer->AddSpacer(10);
    bottomSizer->Add(
        createContinuousScanSettings(this),
        wxSizerFlags(1).Expand()
    );
    bottomSizer->AddSpacer(10);

    sizer->AddSpacer(10);
    sizer->Add(topSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(bottomSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);

    SetSizerAndFit(sizer);
    bindEvents();
}

void BladeAwarenessDlg::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        if (event.CanVeto()) {
            Hide();
            event.Veto();
        } else event.Skip();
    });
}

wxSizer *BladeAwarenessDlg::createIDSetup(wxWindow *parent) {
    auto& config{mParent->getOpenConfig()};

    auto *setupSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Blade ID Setup")
    )};

    auto *enableID{new PCUI::CheckBox(
        setupSizer->GetStaticBox(),
        config.settings.bladeID.enable,
        0,
        _("Enable Blade ID")
    )};
    enableID->SetToolTip(_("Detect when a specific blade is inserted based on a resistor placed in the blade to give it an identifier."));

    auto *mode{new PCUI::Choice(
        setupSizer->GetStaticBox(),
        config.settings.bladeID.mode,
        _("Blade ID Mode")
    )};
    mode->SetToolTip(_("The mode to be used for Blade ID.\nSee the POD page \"Blade ID\" for more info."));

    auto *idPin{new PCUI::ComboBox(
        setupSizer->GetStaticBox(),
        config.settings.bladeID.pin,
        _("Blade ID Pin")
    )};
    idPin->SetToolTip(_("The pin used to detect blade resistance values.\nCannot be the same as Detect Pin."));

    auto *pullupResistance{new PCUI::Numeric(
        setupSizer->GetStaticBox(),
        config.settings.bladeID.pullup,
        _("Pullup Resistance")
    )};
    pullupResistance->SetToolTip(_("The value of the pullup resistor placed on the Blade ID line."));
    auto *pullupPin{new PCUI::ComboBox(
        setupSizer->GetStaticBox(),
        config.settings.bladeID.bridgePin,
        _("Pullup Pin")
    )};
    pullupPin->SetToolTip(_("The pin number or name of the pin which ID Pin is bridged to for pullup.\n This pin cannot be used for anything else."));

    setupSizer->Add(enableID);
    setupSizer->AddSpacer(5);
    setupSizer->Add(mode, wxSizerFlags().Expand());
    setupSizer->AddSpacer(5);
    setupSizer->Add(idPin, wxSizerFlags().Expand());
    setupSizer->Add(
        pullupResistance,
        wxSizerFlags().Expand().Border(wxTOP, 5)
    );
    setupSizer->Add(
        pullupPin,
        wxSizerFlags().Expand().Border(wxTOP, 5)
    );

    return setupSizer;
}

wxSizer *BladeAwarenessDlg::createIDPowerSettings(wxWindow *parent) {
    auto& config{mParent->getOpenConfig()};

    auto *powerForIDSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Power for Blade ID")
    )};

    auto *enablePowerForID {new PCUI::CheckBox(
        powerForIDSizer->GetStaticBox(),
        config.settings.bladeID.powerForID,
        0,
        _("Enable Power on ID")
    )};
    enablePowerForID->SetToolTip(_("Enable power during Blade ID.\nThis is required for WS281X blades."));

    auto *powerPins{new PCUI::CheckList(
        powerForIDSizer->GetStaticBox(),
        config.settings.bladeID.powerPins
    )};

    auto *powerPinEntry{new PCUI::Text(
        powerForIDSizer->GetStaticBox(),
        config.settings.bladeID.powerPinEntry,
        wxTE_PROCESS_ENTER
    )};

    powerForIDSizer->Add(enablePowerForID);
    powerForIDSizer->AddSpacer(5);
    powerForIDSizer->Add(powerPins, wxSizerFlags().Expand());
    powerForIDSizer->AddSpacer(5);
    powerForIDSizer->Add(powerPinEntry, wxSizerFlags().Expand());

    return powerForIDSizer;
}

wxSizer *BladeAwarenessDlg::createContinuousScanSettings(wxWindow *parent) {
    auto& config{mParent->getOpenConfig()};

    auto *continuousScansSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Continuous Scanning")
    )};
    auto *continuousScans{new PCUI::CheckBox(
        continuousScansSizer->GetStaticBox(),
        config.settings.bladeID.continuousScanning,
        0,
        _("Enable Continuous Scanning")
    )};
    continuousScans->SetToolTip(_("Continuously monitor the Blade ID to detect changes."));

    auto *numIDTimes{new PCUI::Numeric(
        continuousScansSizer->GetStaticBox(),
        config.settings.bladeID.continuousTimes,
        _("Number of Reads to Average")
    )};
    numIDTimes->SetToolTip(_("Number of times to read the Blade ID to average out the result and compensate for inaccurate readings."));

    auto *scanIDMillis{new PCUI::Numeric(
        continuousScansSizer->GetStaticBox(),
        config.settings.bladeID.continuousInterval, 
        _("Scan Interval (ms)")
    )};
    scanIDMillis->SetToolTip(_("Scan the Blade ID and update accordingly every input number of millis."));

    continuousScansSizer->Add(continuousScans);
    continuousScansSizer->AddSpacer(5);
    continuousScansSizer->Add(numIDTimes, wxSizerFlags().Expand());
    continuousScansSizer->AddSpacer(5);
    continuousScansSizer->Add(scanIDMillis, wxSizerFlags().Expand());

    return continuousScansSizer;
}

wxSizer *BladeAwarenessDlg::createBladeDetect(wxWindow *parent) {
    auto& config{mParent->getOpenConfig()};

    auto *bladeDetectSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Blade Detect")
    )};
    auto *enableDetect{new PCUI::CheckBox(
        bladeDetectSizer->GetStaticBox(),
        config.settings.bladeDetect,
        0,
        _("Enable Blade Detect")
    )};
    enableDetect->SetToolTip(_("Detect when a blade is inserted into the saber or not."));

    auto *detectPin{new PCUI::ComboBox(
        bladeDetectSizer->GetStaticBox(),
        config.settings.bladeDetectPin,
        _("Blade Detect Pin")
    )};
    detectPin->SetToolTip(_("The pin which will be bridged to BATT- when blade is inserted.\nCannot be the same as ID Pin."));

    bladeDetectSizer->Add(enableDetect, wxSizerFlags().Expand());
    bladeDetectSizer->AddSpacer(5);
    bladeDetectSizer->Add(detectPin, wxSizerFlags().Expand());

    return bladeDetectSizer;
}

