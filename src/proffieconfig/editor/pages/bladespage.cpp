#include "bladespage.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/bladespage.cpp
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

#include <wx/gdicmn.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/tooltip.h>

#include "ui/controls/button.h"
#include "ui/controls/checklist.h"
#include "ui/message.h"
#include "ui/notifier.h"
#include "ui/static_box.h"
#include "utils/image.h"
#include "config/bladeconfig/bladeconfig.h"

#include "../editorwindow.h"
#include "../dialogs/awarenessdlg.h"
#include "../../core/defines.h"
#include "../special/splitvisualizer.h"

class ArrayEditDlg : public wxDialog, PCUI::NotifyReceiver {
public:
    ArrayEditDlg(
        wxWindow *parent,
        Config::BladeConfig& bladeConfig,
        const wxString& title
    ) : wxDialog(parent, wxID_ANY, title),
        NotifyReceiver(this, bladeConfig.notifyData),
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
        nameEntry->SetToolTip(_("The name of the blade array.\nEach name must be unique, but it is for reference only (and thus specific names will not make a difference)."));

        auto *bitsSizer{new wxBoxSizer(wxHORIZONTAL)};

        auto *idEntry{new PCUI::Numeric(
            this,
            bladeConfig.id,
            _("Blade ID")
        )};
        idEntry->SetToolTip(_("The ID of the blade associated with the currently-selected blade array.\nThis value can be measured by typing \"id\" into the Serial Monitor."));
        idEntry->SetMinSize({100, -1});

        constexpr cstring BLADE_ID_TEXT{"NO_BLADE"};
        auto *noBladeID{new PCUI::Toggle(
            this,
            bladeConfig.noBladeID,
            BLADE_ID_TEXT,
            BLADE_ID_TEXT
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

    void handleNotification(uint32) override {
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
    NotifyReceiver(this, parent->getOpenConfig().bladeArrays.notifyData),
    mParent{parent} {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    mAwarenessDlg = new BladeAwarenessDlg(parent);

    sizer->Add(createBladeSelect(), wxSizerFlags(0).Expand());
    sizer->AddSpacer(10);
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
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        ArrayEditDlg dlg(
            mParent,
            bladeArrays.array(bladeArrays.arraySelection),
            _("Edit Blade Array")
        );
        dlg.GetSizer()->AddSpacer(10);
        dlg.buildDone();
        dlg.ShowModal();        
    }, ID_EditArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        
        ArrayEditDlg dlg(
            mParent,
            bladeArrays.addArray(),
            _("Add Blade Array")
        );
        wxSizerFlags flags;
        flags.Expand();
#       ifndef __WXOSX__
        flags.Border(wxALL, 10);
#       endif
        dlg.GetSizer()->Add(
            dlg.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
            flags
        );
        dlg.buildDone();

        if (wxID_OK == dlg.ShowModal()) {
            bladeArrays.arraySelection = static_cast<int32>(bladeArrays.arraySelection.choices().size() - 1);
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
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        bladeArrays.removeArray(bladeArrays.arraySelection);
    }, ID_RemoveArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        bladeArrays.array(bladeArrays.arraySelection).addBlade();
    }, ID_AddBlade);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        auto& array{bladeArrays.array(bladeArrays.arraySelection)};
        array.removeBlade(array.bladeSelection);
    }, ID_RemoveBlade);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        mParent->getOpenConfig().bladeArrays.addPowerPinFromEntry();
    }, ID_PinNameAdd);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        if (config.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{config.bladeArrays.array(config.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Config::Blade::WS281X) return;
        selectedBlade.ws281x().addSplit();
        selectedBlade.ws281x().splitSelect = static_cast<int32>(selectedBlade.ws281x().splits().size() - 1);
    }, ID_AddSplit);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        if (config.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{config.bladeArrays.array(config.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Config::Blade::WS281X) return;
        if (selectedBlade.ws281x().splitSelect == -1) return;
        selectedBlade.ws281x().removeSplit(selectedBlade.ws281x().splitSelect);
    }, ID_RemoveSplit);
}

void BladesPage::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    auto& bladeArrays{mParent->getOpenConfig().bladeArrays};

    if (rebound or id == Config::BladeArrays::ID_ARRAY_SELECTION) {
        bool hasSelection{bladeArrays.arraySelection != -1};
        FindWindow(ID_EditArray)->Enable(hasSelection);
        FindWindow(ID_RemoveArray)->Enable(hasSelection);
        FindWindow(ID_AddBlade)->Enable(hasSelection);
    }
    if (rebound or id == Config::BladeArrays::ID_ARRAY_SELECTION or id == Config::BladeArrays::ID_ARRAY_ISSUES) {
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
            id == Config::BladeArrays::ID_ARRAY_SELECTION or
            id == Config::BladeArrays::ID_BLADE_SELECTION or
            id == Config::BladeArrays::ID_BLADE_TYPE_SELECTION
       ) {
        bool hasSelection{
            bladeArrays.bladeSelectionProxy.data() and
            *bladeArrays.bladeSelectionProxy.data() != -1
        };
        bool hasTypeSelection{
            bladeArrays.bladeTypeProxy.data() and
            *bladeArrays.bladeTypeProxy.data() != -1
        };
        bool isSimple{
            hasTypeSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::SIMPLE
        };
        bool isPixel{
            hasTypeSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::WS281X
        };

        FindWindow(ID_RemoveBlade)->Enable(hasSelection);

        mSimpleSizer->Show(isSimple);
        mPixelSizer->Show(isPixel);

        FindWindow(ID_NoSelectText)->Show(not hasSelection);
    }
    if (
            rebound or 
            id == Config::BladeArrays::ID_ARRAY_SELECTION or
            id == Config::BladeArrays::ID_BLADE_SELECTION or
            id == Config::BladeArrays::ID_BLADE_TYPE_SELECTION or
            id == Config::BladeArrays::ID_SPLIT_SELECTION
       ) {
        auto hasSelection{
            bladeArrays.splitSelectionProxy.data() and
            *bladeArrays.splitSelectionProxy.data() != -1
        };
        FindWindow(ID_RemoveSplit)->Enable(hasSelection);
    }

    if (not rebound) {
        Layout();
        auto size{GetSize()};
        auto bestSize{GetBestSize()};
        if (not size.IsAtLeast(bestSize)) {
            SetMinSize(bestSize);
            mParent->fitAnimated();
        }
    }
}

wxSizer *BladesPage::createBladeSelect() {
    auto& config{mParent->getOpenConfig()};

    auto *bladeSelectSizer{new wxBoxSizer(wxVERTICAL)};
    bladeSelectSizer->SetMinSize(200, -1);

    auto *arraySizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *arraySelection{new PCUI::Choice(
        this,
        config.bladeArrays.arraySelection,
        _("Blade Array")
    )};
    arraySelection->SetToolTip(_("The currently-selected Blade Array to edit."));
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
        config.bladeArrays.bladeSelectionProxy,
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

    bladeManagerSizer->Add(bladeSelectionSizer, wxSizerFlags(1).Expand());

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
    auto& config{mParent->getOpenConfig()};

    auto *settingsSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *noSelectText(new wxStaticText(
        this,
        ID_NoSelectText,
        "No Blade Selected",
        wxDefaultPosition,
        wxSize(350, -1),
        wxALIGN_CENTER
    ));
    noSelectText->SetFont(noSelectText->GetFont().MakeLarger());

    auto *setupSizer{new wxBoxSizer(wxVERTICAL)};

    auto *bladeType{new PCUI::Choice(
        this,
        config.bladeArrays.bladeTypeProxy,
        _("Blade Type")
    )};

    auto starSizer{[this](
            Config::BladeArrays::StarProxy& starProxy,
            const wxString& label
        ) {
        auto *starSizer{new PCUI::StaticBox(
            wxVERTICAL,
            this,
            label
        )};
        auto *starColor{new PCUI::Choice(
            starSizer->childParent(),
            starProxy.ledProxy
        )};
        auto *starPowerPin{new PCUI::ComboBox(
            starSizer->childParent(),
            starProxy.powerPinProxy,
            _("Power Pin")
        )};
        auto *starResistance{new PCUI::Numeric(
            starSizer->childParent(),
            starProxy.resistanceProxy,
            _("Resistance (mOhms)")
        )};
        starResistance->SetToolTip(_("The value of the resistor placed in series with this led."));

        starSizer->Add(starColor, wxSizerFlags().Expand());
        starSizer->AddSpacer(10);
        starSizer->Add(
            starPowerPin, 
            wxSizerFlags().Border(wxLEFT, 20).Expand()
        );
        starSizer->AddSpacer(5);
        starSizer->Add(
            starResistance,
            wxSizerFlags().Border(wxLEFT, 20).Expand()
        );

        return starSizer;
    }};

    mSimpleSizer = new wxBoxSizer(wxVERTICAL);
    auto *simpleSplit1Sizer{new wxBoxSizer(wxHORIZONTAL)};
    simpleSplit1Sizer->Add(starSizer(config.bladeArrays.star1Proxy, _("LED 1")));
    simpleSplit1Sizer->AddSpacer(10);
    simpleSplit1Sizer->Add(starSizer(config.bladeArrays.star2Proxy, _("LED 2")));
    auto *simpleSplit2Sizer{new wxBoxSizer(wxHORIZONTAL)};
    simpleSplit2Sizer->Add(starSizer(config.bladeArrays.star3Proxy, _("LED 3")));
    simpleSplit2Sizer->AddSpacer(10);
    simpleSplit2Sizer->Add(starSizer(config.bladeArrays.star4Proxy, _("LED 4")));
    auto *simpleBrightness{new PCUI::Numeric(
        this,
        config.bladeArrays.simpleBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};
    mSimpleSizer->Add(simpleSplit1Sizer);
    mSimpleSizer->AddSpacer(10);
    mSimpleSizer->Add(simpleSplit2Sizer);
    mSimpleSizer->AddSpacer(10);
    mSimpleSizer->Add(simpleBrightness);

    mPixelSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *pixelMainSizer{new wxBoxSizer(wxVERTICAL)};
    auto *length{new PCUI::Numeric(
        this,
        config.bladeArrays.lengthProxy,
        _("Number of Pixels"),
        wxHORIZONTAL
    )};
    length->SetToolTip(_("The number of pixels in your blade (total)."));
    auto *dataPin{new PCUI::ComboBox(
        this,
        config.bladeArrays.dataPinProxy,
        _("Blade Data Pin"),
        wxHORIZONTAL
    )};
    dataPin->SetToolTip(_("The pin name or number used for WS281X data.\nSpecify custom pins by typing in this box."));
    auto *colorOrder3{new PCUI::Choice(
        this,
        config.bladeArrays.colorOrder3Proxy,
        _("Color Order"),
        wxHORIZONTAL
    )};
    colorOrder3->SetToolTip(_("The order of colors for your blade.\nMost of the time this can be left as \"GRB\"."));
    auto *colorOrder4{new PCUI::Choice(
        this,
        config.bladeArrays.colorOrder4Proxy,
        _("Color Order"),
        wxHORIZONTAL
    )};
    colorOrder4->SetToolTip(_("The order of colors for your blade.\nMost of the time this can be left as \"GRBW\"."));
    auto *hasWhite{new PCUI::CheckBox(
        this,
        config.bladeArrays.hasWhiteProxy,
        0,
        _("LEDs Have White Channel")
    )};
    auto *whiteUseRGB{new PCUI::CheckBox(
        this,
        config.bladeArrays.useRGBWithWhiteProxy,
        0,
        _("Use RGB with White")
    )};
    whiteUseRGB->SetToolTip(_("Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white."));

    auto *pixelBrightness{new PCUI::Numeric(
        this,
        config.bladeArrays.pixelBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};

    auto *pixelPowerPins{new PCUI::CheckList(
        this,
        config.bladeArrays.powerPinProxy,
        _("Power Pins")
    )};
    pixelPowerPins->SetMinSize(wxSize(-1, 200));
    pixelPowerPins->SetToolTip(_("The power pins to use for this blade.\nWS281X blades can have as many as are desired (though 2 is generally enough for most blades)"));

    auto *pinNameSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *powerPinName{new PCUI::Text(
        this,
        config.bladeArrays.powerPinNameEntry,
        wxTE_PROCESS_ENTER,
        false,
        _("Pin Name")
    )};
    auto *addPowerPin{new wxButton(
        this,
        ID_PinNameAdd,
        "+",
        wxDefaultPosition,
        wxSize(30, 20),
        wxBU_EXACTFIT
    )};
    pinNameSizer->Add(powerPinName, wxSizerFlags(1));
    pinNameSizer->Add(addPowerPin, wxSizerFlags().Bottom());

    pixelMainSizer->Add(length, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(dataPin);
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(colorOrder3, wxSizerFlags().Expand());
    pixelMainSizer->Add(colorOrder4, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(hasWhite, wxSizerFlags().Expand());
    pixelMainSizer->Add(
        whiteUseRGB,
        wxSizerFlags().Expand().Border(wxTOP, 5)
    );
    pixelMainSizer->AddSpacer(10);
    pixelMainSizer->Add(pixelBrightness, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(10);
    pixelMainSizer->Add(pixelPowerPins, wxSizerFlags(1).Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(pinNameSizer, wxSizerFlags().Expand());

    auto *pixelSplitSizer{new wxBoxSizer(wxVERTICAL)};
    auto *splitSelect{new PCUI::Choice(
        this,
        config.bladeArrays.splitSelectionProxy,
        _("SubBlades")
    )};
    auto *splitButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addSplit{new wxButton(
        this,
        ID_AddSplit,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removeSplit{new wxButton(
        this,
        ID_RemoveSplit,
        "-",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    splitButtonSizer->Add(addSplit, wxSizerFlags(1));
    splitButtonSizer->AddSpacer(5);
    splitButtonSizer->Add(removeSplit, wxSizerFlags(1));

    auto *splitType{new PCUI::Radios(
        this,
        config.bladeArrays.splitTypeProxy,
        { _("Standard"), _("Reverse"), _("Stride"), _("ZigZag"), _("List") },
        _("Type")
    )};
    splitType->SetToolTip(_(
        "Standard: Split data into continuous sections.\n\n"
        "Reverse: Identical to Standard, but reverses the bladestyle direction.\n\n"
        "Stride: Useful for KR style blades where the data signal \"strides\" back and forth across sides.\n\n"
        "ZigZag: Similar to Stride, but organizes data in the opposite manner perpendicular to the data signal.\n\n"
        "List: Discrete LEDs to make part of a SubBlade."
    ));
    
    auto *splitStartEndSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *splitStart{new PCUI::Numeric(
        this,
        config.bladeArrays.splitStartProxy,
        _("Start")
    )};
    auto *splitEnd{new PCUI::Numeric(
        this,
        config.bladeArrays.splitEndProxy,
        _("End")
    )};
    splitStartEndSizer->Add(splitStart, wxSizerFlags(1));
    splitStartEndSizer->AddSpacer(5);
    splitStartEndSizer->Add(splitEnd, wxSizerFlags(1));

    auto *splitLength{new PCUI::Numeric(
        this,
        config.bladeArrays.splitLengthProxy,
        _("Length")
    )};
    auto *splitSegments{new PCUI::Numeric(
        this,
        config.bladeArrays.splitSegmentsProxy,
        _("Segments")
    )};
    splitSegments->SetToolTip(_("Stride length or number of ZigZag columns"));
    auto *splitList{new PCUI::Text(
        this,
        config.bladeArrays.splitListProxy,
        0,
        false,
        _("Pixel List")
    )};
    splitList->SetToolTip(_("Data goes along each LED according to their order in the list"));
    auto *splitBrightness{new PCUI::Numeric(
        this,
        config.bladeArrays.splitBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};

    pixelSplitSizer->Add(splitSelect, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(5);
    pixelSplitSizer->Add(splitButtonSizer, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(10);
    pixelSplitSizer->Add(splitType, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(10);

    pixelSplitSizer->Add(splitStartEndSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
    pixelSplitSizer->Add(splitLength, wxSizerFlags().Expand());

    pixelSplitSizer->Add(splitSegments, wxSizerFlags().Expand().Border(wxTOP, 10));

    pixelSplitSizer->Add(splitList, wxSizerFlags().Expand());

    pixelSplitSizer->AddSpacer(10);
    pixelSplitSizer->Add(splitBrightness, wxSizerFlags().Expand());

    mPixelSizer->Add(pixelMainSizer, wxSizerFlags().Expand());
    mPixelSizer->AddSpacer(10);
    mPixelSizer->Add(
        new SplitVisualizer(this, config.bladeArrays),
        wxSizerFlags().Expand()
    );
    mPixelSizer->AddSpacer(10);
    mPixelSizer->Add(pixelSplitSizer, wxSizerFlags(1).Expand());

    setupSizer->Add(bladeType);
    setupSizer->AddSpacer(10);
    setupSizer->Add(mSimpleSizer, wxSizerFlags(1).Expand());
    setupSizer->Add(mPixelSizer, wxSizerFlags(1).Expand());

    settingsSizer->Add(noSelectText, wxSizerFlags().Center());
    settingsSizer->Add(setupSizer, wxSizerFlags(1).Expand());

    return settingsSizer;
}
