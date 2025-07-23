#include "bladespage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include <wx/gdicmn.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/tooltip.h>

#include "../editorwindow.h"
#include "../dialogs/awarenessdlg.h"
#include "../../core/defines.h"
#include "ui/controls/checklist.h"

BladesPage::BladesPage(EditorWindow *parent) : 
    wxStaticBoxSizer(wxHORIZONTAL, parent),
    mParent{static_cast<EditorWindow*>(parent)} {

    mAwarenessDlg = new BladeAwarenessDlg(parent);

    Add(createBladeSelect(), wxSizerFlags(0).Expand());
    Add(createBladeSetup(), wxSizerFlags(1).Expand());
    Add(createBladeSettings(), wxSizerFlags(0).ReserveSpaceEvenIfHidden());

    bindEvents();
}

void BladesPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (mAwarenessDlg->IsShown()) mAwarenessDlg->Raise();
        else mAwarenessDlg->Show();
    }, ID_OpenBladeAwareness);
}

// void BladesPage::createToolTips() const {
//     TIP(bladeArray, _("The currently-selected Blade Array to edit."));
//     TIP(addBladeButton, _("Add a blade to the selected blade array."));
//     TIP(removeBladeButton, _("Remove the currently-selected blade."));
//     TIP(addSubBladeButton, _("Add a Sub-Blade to the currently-selected blade.\nCan only be used with WS281X-type blades."));
//     TIP(removeSubBladeButton, _("Remove the currently-selected Sub-Blade.\nIf there are less than 2 Sub-Blades after removal, the remaining Sub-Blade will be deleted."));
// 
//     TIP(bladeType, _("The type of blade/LED."));
//     TIP(powerPins, _("The power pins to use for this blade.\nWS281X blades can have as many as are desired (though 2 is generally enough for most blades)"));
//     TIP(blade3ColorOrder, _("The order of colors for your blade.\nMost of the time this can be left as \"GRB\"."));
//     TIP(blade4ColorOrder, _("The order of colors for your blade.\nMost of the time this can be left as \"GRBW\"."));
//     TIP(blade4UseRGB, _("Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white."));
//     TIP(bladeDataPin, _("The pin name or number used for WS281X data.\nSpecify custom pins by typing in this box."));
//     TIP(bladePixels, _("The number of pixels in your blade (total)."));
// 
//     TIP(useStandard, _("Split apart data into continuous given sections."));
//     TIP(useStride, _("Useful to KR style blades and other similar types where the data signal \"strides\" back and forth across sides."));
//     TIP(useZigZag, _("Similar to using stride, but for blades in which the data is continuous, \"zig-zagging\" up and down the blade."));
//     TIP(subBladeStart, _("The starting pixel number for the current Sub-Blade.\nThis number starts at 0."));
//     TIP(subBladeEnd, _("The ending pixel number for the current Sub-Blade.\nThis number should not exceed the \"Number of Pixels\" in the blade."));
// 
// 
//     TIP(star1Color, _("The profile/type of the first LED.\nCorresponds to the first-selected power pin."));
//     TIP(star2Color, _("The profile/type of the second LED.\nCorresponds to the second-selected power pin."));
//     TIP(star3Color, _("The profile/type of the third LED.\nCorresponds to the third-selected power pin."));
//     TIP(star4Color, _("The profile/type of the fourth LED.\nCorresponds to the fourth-selected power pin."));
//     TIP(star1Resistance, _("The value of the resistor placed in series with this led."));
//     TIP(star2Resistance, _("The value of the resistor placed in series with this led."));
//     TIP(star3Resistance, _("The value of the resistor placed in series with this led."));
//     TIP(star4Resistance, _("The value of the resistor placed in series with this led."));
// }

wxBoxSizer *BladesPage::createBladeSelect() {
    auto config{mParent->getOpenConfig()};

    auto *bladeSelectSizer{new wxBoxSizer(wxVERTICAL)};
    auto *bladeArray{new PCUI::Choice(
        GetStaticBox(),
        config->bladeArrays.arraySelection,
        _("Blade Array")
    )};
    auto *bladeAwarenessButton{new wxButton(
        GetStaticBox(),
        ID_OpenBladeAwareness,
        _("Blade Awareness...")
    )};
    bladeSelectSizer->Add(bladeArray, TEXTITEMFLAGS.Expand());
    bladeSelectSizer->Add(
        bladeAwarenessButton,
        wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 5).Expand()
    );
    bladeSelectSizer->Add(
        createBladeManager(),
        wxSizerFlags(1).Border(wxALL, 5).Expand()
    );

    return bladeSelectSizer;
}

wxBoxSizer *BladesPage::createBladeManager() {
    auto config{mParent->getOpenConfig()};

    auto *bladeManagerSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *bladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    auto *bladeSelect{new PCUI::List(
        GetStaticBox(),
        config->bladeArrays.bladeSelectionProxy,
        _("Blades")
    )};
    auto *bladeButtons{new wxBoxSizer(wxHORIZONTAL)};
    auto *addBladeButton{new wxButton(
        GetStaticBox(),
        ID_AddBlade,
        "+",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    auto *removeBladeButton{new wxButton(
        GetStaticBox(),
        ID_RemoveBlade,
        "-",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    bladeButtons->Add(addBladeButton);
    bladeButtons->AddSpacer(10);
    bladeButtons->Add(removeBladeButton);

    bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand());
    bladeSelectionSizer->AddSpacer(5);
    bladeSelectionSizer->Add(bladeButtons, wxSizerFlags(0).Center());

    auto *subBladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    auto *subBladeSelect{new PCUI::List(
        GetStaticBox(),
        config->bladeArrays.subBladeSelectionProxy,
        _("SubBlades")
    )};
    auto *subBladeButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addSubBladeButton{new wxButton(
        GetStaticBox(),
        ID_AddSubBlade,
        "+",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    auto *removeSubBladeButton{new wxButton(
        GetStaticBox(),
        ID_RemoveSubBlade,
        "-",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    subBladeButtonSizer->Add(addSubBladeButton);
    bladeButtons->AddSpacer(10);
    subBladeButtonSizer->Add(removeSubBladeButton);
    subBladeSelectionSizer->Add(subBladeSelect, wxSizerFlags(1).Expand());
    subBladeSelectionSizer->AddSpacer(5);
    subBladeSelectionSizer->Add(subBladeButtonSizer, wxSizerFlags(0).Center());

    bladeManagerSizer->Add(bladeSelectionSizer, wxSizerFlags(1).Expand());
    bladeManagerSizer->Add(subBladeSelectionSizer, wxSizerFlags(1).Expand());

    return bladeManagerSizer;
}

wxBoxSizer *BladesPage::createBladeSetup() {
    auto config{mParent->getOpenConfig()};

    auto *bladeSetup{new wxBoxSizer(wxVERTICAL)};
    auto *bladeType{new PCUI::Choice(
        GetStaticBox(),
        config->bladeArrays.bladeTypeProxy,
        _("Blade Type")
    )};
    auto *powerPins{new PCUI::CheckList(
        GetStaticBox(),
        config->bladeArrays.powerPinProxy
    )};
    powerPins->SetMinSize(wxSize(200, -1));

    auto *pinNameSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addPowerPin{new wxButton(
        GetStaticBox(),
        ID_AddPowerPinSelection,
        "+",
        wxDefaultPosition,
        wxSize(30, 20),
        wxBU_EXACTFIT
    )};
    auto *powerPinName{new PCUI::Text(
        GetStaticBox(),
        config->bladeArrays.powerPinNameEntry,
        0,
        _("Pin Name")
    )};
    pinNameSizer->Add(
        powerPinName,
        wxSizerFlags(1).Border(wxRIGHT, 5)
    );
    pinNameSizer->Add(addPowerPin, wxSizerFlags(0).Bottom());

    bladeSetup->Add(
        bladeType,
        wxSizerFlags(0)
            .Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand()
    );
    bladeSetup->Add(
        powerPins,
        wxSizerFlags(1)
            .Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand()
    );
    bladeSetup->Add(
        pinNameSizer,
        wxSizerFlags(0)
            .Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand()
    );

    return bladeSetup;
}

wxBoxSizer *BladesPage::createBladeSettings() {
    auto config{mParent->getOpenConfig()};

    auto *bladeSettings{new wxBoxSizer(wxVERTICAL)};
    auto *bladeColor{new wxBoxSizer(wxVERTICAL)};
    auto *blade3ColorOrder{new PCUI::Choice(
        GetStaticBox(),
        config->bladeArrays.colorOrder3Proxy,
        _("Color Order")
    )};
    auto *blade4ColorOrder{new PCUI::Choice(
        GetStaticBox(),
        config->bladeArrays.colorOrder4Proxy,
        _("Color Order")
    )};
    bladeColor->Add(
        blade3ColorOrder,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );
    bladeColor->Add(
        blade4ColorOrder,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );

    auto *blade4UseRGB{new PCUI::CheckBox(
        GetStaticBox(),
        config->bladeArrays.useRGBWithWhiteProxy,
        0,
        _("Use RGB with White")
    )};
    auto *bladeDataPin{new PCUI::ComboBox(
        GetStaticBox(),
        config->bladeArrays.dataPinProxy,
        _("Blade Data Pin")
    )};
    auto *bladePixels{new PCUI::Numeric(
        GetStaticBox(),
        config->bladeArrays.lengthProxy,
        wxSP_ARROW_KEYS,
        _("Number of Pixels")
    )};

    auto starSizer{[this](
            Config::BladeArrays::StarProxy& starProxy,
            const wxString& label
        ) {
        auto *starSizer{new wxStaticBoxSizer(
            wxVERTICAL,
            GetStaticBox(),
            label
        )};
        auto *starColor{new PCUI::Choice(
            starSizer->GetStaticBox(),
            starProxy.ledProxy
        )};
        auto *starPowerPin{new PCUI::ComboBox(
            starSizer->GetStaticBox(),
            starProxy.powerPinProxy,
            _("Power Pin")
        )};
        auto *starResistance{new PCUI::Numeric(
            starSizer->GetStaticBox(),
            starProxy.resistanceProxy,
            wxSP_ARROW_KEYS,
            _("Resistance (mOhms)")
        )};
        starSizer->Add(
            starColor,
            wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5)
        );
        starSizer->Add(
            starPowerPin, 
            wxSizerFlags(0)
                .Border(wxLEFT | wxRIGHT, 5)
                .TripleBorder(wxLEFT)
                .Expand()
        );
        starSizer->AddSpacer(5);
        starSizer->Add(
            starResistance,
            wxSizerFlags(0)
                .Border(wxLEFT | wxRIGHT, 5)
                .TripleBorder(wxLEFT)
                .Expand()
        );

        return starSizer;
    }};

    auto *subBladeType{new PCUI::Radios(
        GetStaticBox(),
        config->bladeArrays.subBladeTypeProxy,
        _("SubBlade Type")
    )};

    auto *splitLength{new PCUI::Numeric(
        GetStaticBox(),
        config->bladeArrays.subBladeLengthProxy,
        wxSP_ARROW_KEYS,
        _("SubBlade Start")
    )};
    auto *splitSegments{new PCUI::Numeric(
        GetStaticBox(),
        config->bladeArrays.subBladeSegmentsProxy,
        wxSP_ARROW_KEYS,
        _("SubBlade End")
    )};

    bladeSettings->Add(bladeColor);
    bladeSettings->Add(blade4UseRGB, MENUITEMFLAGS);
    bladeSettings->Add(
        starSizer(config->bladeArrays.star1Proxy, _("LED 1")),
        MENUITEMFLAGS.DoubleBorder(wxBOTTOM)
    );
    bladeSettings->Add(
        starSizer(config->bladeArrays.star2Proxy, _("LED 2")),
        MENUITEMFLAGS.DoubleBorder(wxBOTTOM)
    );
    bladeSettings->Add(
        starSizer(config->bladeArrays.star3Proxy, _("LED 3")),
        MENUITEMFLAGS.DoubleBorder(wxBOTTOM)
    );
    bladeSettings->Add(
        starSizer(config->bladeArrays.star4Proxy, _("LED 4")),
        MENUITEMFLAGS.DoubleBorder(wxBOTTOM)
    );
    bladeSettings->Add(
        bladeDataPin,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );
    bladeSettings->Add(
        bladePixels,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );

    bladeSettings->Add(subBladeType, MENUITEMFLAGS);
    bladeSettings->Add(
        splitLength,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );
    bladeSettings->Add(
        splitSegments,
        wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );

    bladeSettings->SetMinSize(150, -1);
    return bladeSettings;
}
