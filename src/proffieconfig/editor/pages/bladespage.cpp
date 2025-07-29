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
#include "config/bladeconfig/bladeconfig.h"
#include "ui/controls/button.h"
#include "ui/controls/checklist.h"
#include "ui/message.h"
#include "ui/notifier.h"
#include "utils/image.h"
#include "wx/anybutton.h"
#include "wx/settings.h"

class ArrayEditDlg : public wxDialog, PCUI::Notifier {
public:
    ArrayEditDlg(
        wxWindow *parent,
        Config::BladeConfig& bladeConfig,
        const wxString& title
    ) : wxDialog(parent, wxID_ANY, title),
        Notifier(this, bladeConfig.notifyData),
        mBladeConfig{bladeConfig} {
        auto *sizer{new wxBoxSizer(wxVERTICAL)};

        sizer->AddSpacer(10);

        auto *nameEntry{new PCUI::Text(
            this,
            bladeConfig.name,
            0,
            false,
            _("Name")
        )};

        auto *bitsSizer{new wxBoxSizer(wxHORIZONTAL)};

        auto *idEntry{new PCUI::Numeric(
            this,
            bladeConfig.id,
            wxSP_ARROW_KEYS,
            _("Blade ID")
        )};
        idEntry->SetMinSize({100, -1});

        cstring bladeIdText{"NO_BLADE"};
        auto *noBladeID{new PCUI::Toggle(
            this,
            bladeConfig.noBladeID,
            bladeIdText,
            bladeIdText
        )};

        auto *presetEntry{new PCUI::Choice(
            this,
            bladeConfig.presetArray,
            _("Preset Array")
        )};

        bitsSizer->Add(idEntry, wxSizerFlags(1));
        bitsSizer->AddSpacer(5);
        bitsSizer->Add(noBladeID, wxSizerFlags(0).Bottom());
        bitsSizer->AddSpacer(10);
        bitsSizer->Add(presetEntry, wxSizerFlags(2));

        auto *issueText{new wxStaticText(
            this,
            ID_IssueText,
            wxEmptyString
        )};

        sizer->AddSpacer(10);
        sizer->Add(
            nameEntry,
            wxSizerFlags().Border(wxLEFT | wxRIGHT, 10).Expand()
        );
        sizer->AddSpacer(5);
        sizer->Add(
            bitsSizer,
            wxSizerFlags().Border(wxLEFT | wxRIGHT, 10).Expand()
        );
        sizer->AddSpacer(10);

        sizer->Add(
            issueText,
            wxSizerFlags().Right().Border(wxBOTTOM | wxRIGHT, 5)
                .DoubleBorder(wxRIGHT)

        );

        SetSizer(sizer);
    }

    void buildDone() { initializeNotifier(); }

    enum {
        ID_IssueText
    };

private:
    Config::BladeConfig& mBladeConfig;

    void handleNotification(uint32) {
        auto issues{mBladeConfig.computeIssues()};
        auto *okButton{FindWindow(wxID_OK)};
        if (okButton) {
            constexpr auto PERMIT_ISSUES{Config::BladeConfig::ISSUE_NO_PRESETARRAY};
            okButton->Enable((issues & ~PERMIT_ISSUES) == Config::BladeConfig::ISSUE_NONE);
        }

        auto *issueText{FindWindow(ArrayEditDlg::ID_IssueText)};
        issueText->Show(issues != Config::BladeConfig::ISSUE_NONE);
        if (issueText->IsShown()) {
            wxString label;
            if (issues & Config::BladeConfig::ISSUE_DUPLICATE_NAME) {
                label += Config::BladeConfig::issueString(Config::BladeConfig::ISSUE_DUPLICATE_NAME);
            }
            if (issues & Config::BladeConfig::ISSUE_NO_NAME) {
                label += Config::BladeConfig::issueString(Config::BladeConfig::ISSUE_NO_NAME);
            }
            if (issues & Config::BladeConfig::ISSUE_DUPLICATE_ID) {
                if (not label.empty()) label += '\n';
                label += Config::BladeConfig::issueString(Config::BladeConfig::ISSUE_DUPLICATE_ID);
            }
            if (issues & Config::BladeConfig::ISSUE_NO_PRESETARRAY) {
                if (not label.empty()) label += '\n';
                label += Config::BladeConfig::issueString(Config::BladeConfig::ISSUE_NO_PRESETARRAY);
            }
            issueText->SetLabel(label);
        }
        Layout();
        Fit();
    }
};

BladesPage::BladesPage(EditorWindow *parent) : 
    wxPanel(parent),
    Notifier(this, parent->getOpenConfig()->bladeArrays.notifyData),
    mParent{static_cast<EditorWindow*>(parent)} {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    mAwarenessDlg = new BladeAwarenessDlg(parent);

    sizer->Add(createBladeSelect(), wxSizerFlags(0).Expand());
    sizer->Add(createBladeSettings(), wxSizerFlags(1).Expand());

    bindEvents();
    initializeNotifier();
    SetSizerAndFit(sizer);
}
 
void BladesPage::bindEvents() {
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (mAwarenessDlg->IsShown()) mAwarenessDlg->Raise();
        else mAwarenessDlg->Show();
    }, ID_OpenBladeAwareness);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& bladeArrays{mParent->getOpenConfig()->bladeArrays};
        ArrayEditDlg dlg(
            mParent,
            bladeArrays.array(bladeArrays.arraySelection),
            _("Edit Blade Array")
        );
        dlg.buildDone();
        dlg.ShowModal();        
    }, ID_EditArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& bladeArrays{mParent->getOpenConfig()->bladeArrays};
        
        ArrayEditDlg dlg(
            mParent,
            bladeArrays.addArray(),
            _("Add Blade Array")
        );
        dlg.GetSizer()->Add(
            dlg.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
            wxSizerFlags().Expand()
        );
        dlg.buildDone();

        if (wxID_OK == dlg.ShowModal()) {
            bladeArrays.arraySelection = bladeArrays.arraySelection.choices().size() - 1;
        } else {
            bladeArrays.removeArray(bladeArrays.arraySelection.choices().size() - 1);
        }
    }, ID_AddArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto res{PCUI::showMessage(
            "This action cannot be undone!",
            "Remove Blade Array?",
            wxYES_NO | wxNO_DEFAULT
        )};
        if (res != wxYES) return;
        auto& bladeArrays{mParent->getOpenConfig()->bladeArrays};
        bladeArrays.removeArray(bladeArrays.arraySelection);
    }, ID_RemoveArray);
}

void BladesPage::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    auto& bladeArrays{mParent->getOpenConfig()->bladeArrays};

    if (rebound or id == bladeArrays.ID_ARRAY_SELECTION) {
        bool hasSelection{bladeArrays.arraySelection != -1};
        FindWindow(ID_EditArray)->Enable(hasSelection);
        FindWindow(ID_RemoveArray)->Enable(hasSelection);
        FindWindow(ID_AddBlade)->Enable(hasSelection);
        FindWindow(ID_RemoveBlade)->Enable(hasSelection);
    }
    if (rebound or id == bladeArrays.ID_ARRAY_SELECTION or id == bladeArrays.ID_ARRAY_ISSUES) {
        const auto issues{bladeArrays.arrayIssues};

        auto *issueIcon{FindWindow(ID_IssueIcon)};
        if (issues & Config::BladeConfig::ISSUE_ERRORS) {
            issueIcon->SetLabel(L"\u26D4" /* ⛔️ */);

        } else if (issues & Config::BladeConfig::ISSUE_WARNINGS) {
            issueIcon->SetLabel(L"\u26A0" /* ⚠️  */);
        }
        issueIcon->Show(issues != Config::BladeConfig::ISSUE_NONE);
    }
    if (
            rebound or 
            id == bladeArrays.ID_BLADE_SELECTION or
            id == bladeArrays.ID_SUBBLADE_SELECTION or
            id == bladeArrays.ID_BLADE_TYPE_SELECTION
       ) {
        bool hasSelection{
            bladeArrays.bladeTypeProxy.data() and
            *bladeArrays.bladeTypeProxy.data() != -1
        };
        bool isSimple{
            hasSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::SIMPLE
        };
        bool isPixel{
            hasSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::WS281X
        };

        FindWindow(ID_Star1Box)->Show(isSimple);
        FindWindow(ID_Star2Box)->Show(isSimple);
        FindWindow(ID_Star3Box)->Show(isSimple);
        FindWindow(ID_Star4Box)->Show(isSimple);

        FindWindow(ID_PinNameAdd)->Show(isPixel);

        FindWindow(ID_NoSelectText)->Show(not isPixel and not isSimple);
    }

    Layout();
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

wxSizer *BladesPage::createBladeSelect() {
    auto config{mParent->getOpenConfig()};

    auto *bladeSelectSizer{new wxBoxSizer(wxVERTICAL)};
    bladeSelectSizer->SetMinSize(200, -1);

    auto *arraySizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *arraySelection{new PCUI::Choice(
        this,
        config->bladeArrays.arraySelection,
        _("Blade Array")
    )};
    auto *issueIcon{new wxButton(
        this,
        ID_IssueIcon,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *editArrayButton{new PCUI::Button(
        this,
        ID_EditArray,
        wxEmptyString,
        wxDefaultSize,
        wxBU_EXACTFIT,
        "edit",
        {-1, 16},
        wxSYS_COLOUR_WINDOWTEXT

    )};
    arraySizer->Add(arraySelection, wxSizerFlags(1));
    arraySizer->Add(
        issueIcon,
        wxSizerFlags().Border(wxLEFT, 5).Bottom()
    );
    arraySizer->AddSpacer(5);
    arraySizer->Add(editArrayButton, wxSizerFlags().Bottom());

    auto *arrayButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addArrayButton{new wxButton(
        this,
        ID_AddArray,
        _("Add"),
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removeArrayButton{new wxButton(
        this,
        ID_RemoveArray,
        _("Remove"),
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    arrayButtonSizer->Add(addArrayButton, wxSizerFlags(2));
    arrayButtonSizer->AddSpacer(5);
    arrayButtonSizer->Add(removeArrayButton, wxSizerFlags(3));

    auto *bladeAwarenessButton{new wxButton(
        this,
        ID_OpenBladeAwareness,
        _("Blade Awareness...")
    )};

    auto *bladeManagerSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *bladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    auto *bladeSelect{new PCUI::List(
        this,
        config->bladeArrays.bladeSelectionProxy,
        _("Blades")
    )};

    auto *bladeButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addBladeButton{new wxButton(
        this,
        ID_AddBlade,
        "+",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    auto *removeBladeButton{new wxButton(
        this,
        ID_RemoveBlade,
        "-",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    bladeButtonSizer->Add(addBladeButton, wxSizerFlags(1));
    bladeButtonSizer->AddSpacer(5);
    bladeButtonSizer->Add(removeBladeButton, wxSizerFlags(1));

    bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand());
    bladeSelectionSizer->AddSpacer(5);
    bladeSelectionSizer->Add(bladeButtonSizer, wxSizerFlags().Expand());

    // auto *subBladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    // auto *subBladeSelect{new PCUI::List(
    //     GetStaticBox(),
    //     config->bladeArrays.subBladeSelectionProxy,
    //     _("SubBlades")
    // )};
    // auto *subBladeButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    // auto *addSubBladeButton{new wxButton(
    //     GetStaticBox(),
    //     ID_AddSubBlade,
    //     "+",
    //     wxDefaultPosition,
    //     SMALLBUTTONSIZE,
    //     wxBU_EXACTFIT
    // )};
    // auto *removeSubBladeButton{new wxButton(
    //     GetStaticBox(),
    //     ID_RemoveSubBlade,
    //     "-",
    //     wxDefaultPosition,
    //     SMALLBUTTONSIZE,
    //     wxBU_EXACTFIT
    // )};
    // subBladeButtonSizer->Add(addSubBladeButton, wxSizerFlags(1));
    // subBladeButtonSizer->AddSpacer(5);
    // subBladeButtonSizer->Add(removeSubBladeButton, wxSizerFlags(1));

    // subBladeSelectionSizer->Add(subBladeSelect, wxSizerFlags(1).Expand());
    // subBladeSelectionSizer->AddSpacer(5);
    // subBladeSelectionSizer->Add(subBladeButtonSizer, wxSizerFlags().Expand());

    bladeManagerSizer->Add(bladeSelectionSizer, wxSizerFlags(1).Expand());
    // bladeManagerSizer->Add(subBladeSelectionSizer, wxSizerFlags(1).Expand());


    bladeSelectSizer->Add(arraySizer, wxSizerFlags().Expand());
    bladeSelectSizer->AddSpacer(5);
    bladeSelectSizer->Add(arrayButtonSizer, wxSizerFlags().Expand());
    bladeSelectSizer->AddSpacer(10);
    bladeSelectSizer->Add(bladeAwarenessButton, wxSizerFlags().Expand());
    bladeSelectSizer->AddSpacer(10);
    bladeSelectSizer->Add(bladeManagerSizer, wxSizerFlags(1).Expand());

    return bladeSelectSizer;
}

wxSizer *BladesPage::createBladeSettings() {
    auto config{mParent->getOpenConfig()};

    auto *settingsSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *noSelectText(new wxStaticText(
        this,
        ID_NoSelectText,
        "No Blade Selected",
        wxDefaultPosition,
        wxSize(500, -1),
        wxALIGN_CENTER
    ));
    noSelectText->SetFont(noSelectText->GetFont().MakeLarger());

    auto *setupSizer{new wxBoxSizer(wxVERTICAL)};

    auto *bladeType{new PCUI::Choice(
        this,
        config->bladeArrays.bladeTypeProxy,
        _("Blade Type")
    )};

    auto starSizer{[this](
            Config::BladeArrays::StarProxy& starProxy,
            const wxString& label,
            wxWindowID id
        ) {
        auto *starSizer{new wxStaticBoxSizer(
            wxVERTICAL,
            this,
            label
        )};
        starSizer->GetStaticBox()->SetId(id);
        auto *starColor{new PCUI::Choice(
            starSizer->GetStaticBox(), starProxy.ledProxy
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

    auto *simpleSizer1{new wxBoxSizer(wxHORIZONTAL)};
    simpleSizer1->Add(starSizer(config->bladeArrays.star1Proxy, _("LED 1"), ID_Star1Box));
    simpleSizer1->Add(starSizer(config->bladeArrays.star2Proxy, _("LED 2"), ID_Star2Box));
    auto *simpleSizer2{new wxBoxSizer(wxHORIZONTAL)};
    simpleSizer2->Add(starSizer(config->bladeArrays.star3Proxy, _("LED 3"), ID_Star3Box));
    simpleSizer2->Add(starSizer(config->bladeArrays.star4Proxy, _("LED 4"), ID_Star4Box));

    auto pixelSizer1{new wxBoxSizer(wxHORIZONTAL)};
    auto *dataPin{new PCUI::ComboBox(
        this,
        config->bladeArrays.dataPinProxy,
        _("Blade Data Pin")
    )};
    auto *length{new PCUI::Numeric(
        this,
        config->bladeArrays.lengthProxy,
        wxSP_ARROW_KEYS,
        _("Number of Pixels")
    )};
    pixelSizer1->Add(dataPin);
    pixelSizer1->Add(length);

    auto *colorOrder3{new PCUI::Choice(
        this,
        config->bladeArrays.colorOrder3Proxy,
        _("Color Order")
    )};
    auto *colorOrder4{new PCUI::Choice(
        this,
        config->bladeArrays.colorOrder4Proxy,
        _("Color Order")
    )};

    auto *whiteUseRGB{new PCUI::CheckBox(
        this,
        config->bladeArrays.useRGBWithWhiteProxy,
        0,
        _("Use RGB with White")
    )};

    auto *pixelPowerPins{new PCUI::CheckList(
        this,
        config->bladeArrays.powerPinProxy
    )};
    pixelPowerPins->SetMinSize(wxSize(200, -1));

    auto *pinNameSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addPowerPin{new wxButton(
        this,
        ID_PinNameAdd,
        "+",
        wxDefaultPosition,
        wxSize(30, 20),
        wxBU_EXACTFIT
    )};
    auto *powerPinName{new PCUI::Text(
        this,
        config->bladeArrays.powerPinNameEntry,
        0,
        false,
        _("Pin Name")
    )};
    pinNameSizer->Add(powerPinName);
    pinNameSizer->Add(addPowerPin, wxSizerFlags().Bottom());

    setupSizer->Add(bladeType);

    setupSizer->Add(simpleSizer1);
    setupSizer->Add(simpleSizer2);

    setupSizer->Add(pixelSizer1);
    setupSizer->Add(colorOrder3);
    setupSizer->Add(colorOrder4);
    setupSizer->Add(whiteUseRGB);
    setupSizer->Add(pixelPowerPins, wxSizerFlags(1));
    setupSizer->Add(pinNameSizer);

    // auto *subBladeType{new PCUI::Radios(
    //     GetStaticBox(),
    //     config->bladeArrays.subBladeTypeProxy,
    //     _("SubBlade Type")
    // )};

    // auto *splitLength{new PCUI::Numeric(
    //     GetStaticBox(),
    //     config->bladeArrays.subBladeLengthProxy,
    //     wxSP_ARROW_KEYS,
    //     _("SubBlade Start")
    // )};
    // auto *splitSegments{new PCUI::Numeric(
    //     GetStaticBox(),
    //     config->bladeArrays.subBladeSegmentsProxy,
    //     wxSP_ARROW_KEYS,
    //     _("SubBlade End")
    // )};

    // bladeSettings->Add(
    //     bladeDataPin,
    //     wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    // );
    // bladeSettings->Add(
    //     bladePixels,
    //     wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    // );

    // bladeSettings->Add(subBladeType);
    // bladeSettings->Add(
    //     splitLength,
    //     wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    // );
    // bladeSettings->Add(
    //     splitSegments,
    //     wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    // );

    settingsSizer->Add(noSelectText, wxSizerFlags().Center());
    settingsSizer->Add(setupSizer, wxSizerFlags().Expand());

    return settingsSizer;
}
