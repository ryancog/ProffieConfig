#include "bladespage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../pages/generalpage.h"
#include "../dialogs/bladearraydlg.h"
#include "ui/controls.h"
#include "../../core/utilities/misc.h"
#include "../../core/defines.h"
#include "wx/strvararg.h"

#include <cmath>
#include <limits>
#include <wx/gdicmn.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/tooltip.h>

bool BladesPage::hasBladeSelection() const { return bladeSelect->GetSelection() != -1; }
bool BladesPage::hasSubBladeSelection() const { return subBladeSelect->GetSelection() != -1; }
bool BladesPage::isRGBPixelBlade() const { return hasBladeSelection() and bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGB; }
bool BladesPage::isRGBWPixelBlade() const { return hasBladeSelection() and bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGBW; }
bool BladesPage::isPixelBlade() const { return isRGBPixelBlade() or isRGBWPixelBlade(); }
bool BladesPage::isSimpleBlade() const { return hasBladeSelection() and bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_SIMPLE; }
bool BladesPage::isSubBlade() const { return hasBladeSelection() and bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].isSubBlade; }
bool BladesPage::isFirstBlade() const { return not isSubBlade() or subBladeSelect->GetSelection() == 0; }
bool BladesPage::isStandardSubBlade() const { return isSubBlade() and useStandard->GetValue(); }
bool BladesPage::isStrideSubBlade() const { return isSubBlade() and useStride->GetValue(); }
bool BladesPage::isZigZagSubBlade() const { return isSubBlade() and useZigZag->GetValue(); }

wxArrayString BladesPage::ledStrings() {
    return { 
        _("<None>"),
        _("Cree Red"),
        _("Cree Green"),
        _("Cree Blue"),
        _("Cree Amber"),
        _("Cree Red-Orange"),
        _("Cree White"),
        _("Red"),
        _("Green"),
        _("Blue"),
    };
}

wxString BladesPage::ledToStr(LED led) {
    switch (led) {
        case CREE_RED:
            return _("Cree Red");
        case CREE_GREEN:
            return _("Cree Green");
        case CREE_BLUE:
            return _("Cree Blue");
        case CREE_AMBER:
            return _("Cree Amber");
        case CREE_RED_ORANGE:
            return _("Cree Red-Orange");
        case CREE_WHITE:
            return _("Cree White");
        case RED:
            return _("Red");
        case GREEN:
            return _("Green");
        case BLUE:
            return _("Blue");
        case NONE:
        default:
            return _("<None>");
    }
}

BladesPage::LED BladesPage::strToLed(const wxString& str) {
    if (str == _("Cree Red")) return CREE_RED;
    if (str == _("Cree Green")) return CREE_GREEN;
    if (str == _("Cree Blue")) return CREE_BLUE;
    if (str == _("Cree Amber")) return CREE_AMBER;
    if (str == _("Cree Red-Orange")) return CREE_RED_ORANGE;
    if (str == _("Cree White")) return CREE_WHITE;
    if (str == _("Red")) return RED;
    if (str == _("Green")) return GREEN;
    if (str == _("Blue")) return BLUE;

    return NONE;

}

wxString BladesPage::ledToConfigStr(LED led) {
    switch (led) {
        case CREE_RED:
            return "CreeXPE2RedTemplate";
        case CREE_GREEN:
            return "CreeXPE2GreenTemplate";
        case CREE_BLUE:
            return "CreeXPE2BlueTemplate";
        case CREE_AMBER:
            return "CreeXPE2AmberTemplate";
        case CREE_RED_ORANGE:
            return "CreeXPE2RedOrangeTemplate";
        case CREE_WHITE:
            return "CreeXPE2WhiteTemplate";
        case RED:
            return "CH1LED";
        case GREEN:
            return "CH2LED";
        case BLUE:
            return "CH3LED";
        case NONE:
        default:
            return "NoLED";
    }
}

BladesPage::BladesPage(wxWindow *window) : wxStaticBoxSizer(wxHORIZONTAL, window), mParent{static_cast<EditorWindow*>(window)} {
    bladeArrayDlg = new BladeArrayDlg(mParent);

    Add(createBladeSelect(), wxSizerFlags(0).Expand());
    Add(createBladeSetup(), wxSizerFlags(1).Expand());
    Add(createBladeSettings(), wxSizerFlags(0).ReserveSpaceEvenIfHidden());

    bindEvents();
    createToolTips();
}

void BladesPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { if (bladeArrayDlg->IsShown()) bladeArrayDlg->Raise(); else bladeArrayDlg->Show(); }, ID_OpenBladeArrays);

    GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { mParent->presetsPage->bladeArray->entry()->SetSelection(bladeArray->entry()->GetSelection()); update(); }, ID_BladeArray);
    GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { setEnabled(); }, ID_LEDColor);
    GetStaticBox()->Bind(wxEVT_SPINCTRL, [&](wxCommandEvent& event) { update(); event.Skip(); });
    GetStaticBox()->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent& event) { update(); event.Skip(); });
    GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { update(); }, ID_BladeSelect);
    GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { update(); }, ID_SubBladeSelect);
    GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { update(); }, ID_BladeType);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addBlade(); }, ID_AddBlade);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addSubBlade(); }, ID_AddSubBlade);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeBlade(); }, ID_RemoveBlade);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeSubBlade(); }, ID_RemoveSubBlade);
    GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        auto insertionPoint{powerPinName->entry()->GetInsertionPoint()};
        wxString newString{};
        for (char chr : powerPinName->entry()->GetValue().ToStdString()) if (isalnum(chr)) newString += chr;

        powerPinName->entry()->ChangeValue(newString);
        powerPinName->entry()->SetInsertionPoint(insertionPoint < newString.length() ? insertionPoint : static_cast<int32>(newString.length()));
        update();
    }, ID_PowerPinName);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        for (const auto& pin : powerPins->GetStrings()) {
            if (pin == powerPinName->entry()->GetValue()) {
                powerPins->Check(powerPins->FindString(pin));
                powerPinName->entry()->Clear();
                return;
            }
        }

        powerPins->Append(powerPinName->entry()->GetValue());
        powerPins->Check(powerPins->GetCount() - 1);
        powerPinName->entry()->Clear();
        update();
    }, ID_AddPowerPin);
    GetStaticBox()->Bind(wxEVT_CHECKLISTBOX, [&](wxCommandEvent&) {
        update();
        for (uint32_t idx = 6; idx < powerPins->GetCount(); idx++) {
            if (powerPins->IsChecked(idx)) continue;

            bool found{false};
            for (const auto& array : bladeArrayDlg->bladeArrays) {
                if (found) break;
                for (const auto& blade : array.blades) {
                    if (found) break;
                    for (const auto& pin : blade.powerPins) {
                        if (powerPins->GetString(idx) == pin) {
                            found = true;
                            break;
                        }
                    }
                }
            }
            if (found) continue;

            powerPins->Delete(idx);
        }
    }, ID_PowerPins);
}

void BladesPage::createToolTips() const {
    TIP(bladeArray, _("The currently-selected Blade Array to edit."));
    TIP(addBladeButton, _("Add a blade to the selected blade array."));
    TIP(removeBladeButton, _("Remove the currently-selected blade."));
    TIP(addSubBladeButton, _("Add a Sub-Blade to the currently-selected blade.\nCan only be used with WS281X-type blades."));
    TIP(removeSubBladeButton, _("Remove the currently-selected Sub-Blade.\nIf there are less than 2 Sub-Blades after removal, the remaining Sub-Blade will be deleted."));

    TIP(bladeType, _("The type of blade/LED."));
    TIP(powerPins, _("The power pins to use for this blade.\nWS281X blades can have as many as are desired (though 2 is generally enough for most blades)"));
    TIP(blade3ColorOrder, _("The order of colors for your blade.\nMost of the time this can be left as \"GRB\"."));
    TIP(blade4ColorOrder, _("The order of colors for your blade.\nMost of the time this can be left as \"GRBW\"."));
    TIP(blade4UseRGB, _("Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white."));
    TIP(bladeDataPin, _("The pin name or number used for WS281X data.\nSpecify custom pins by typing in this box."));
    TIP(bladePixels, _("The number of pixels in your blade (total)."));

    TIP(useStandard, _("Split apart data into continuous given sections."));
    TIP(useStride, _("Useful to KR style blades and other similar types where the data signal \"strides\" back and forth across sides."));
    TIP(useZigZag, _("Similar to using stride, but for blades in which the data is continuous, \"zig-zagging\" up and down the blade."));
    TIP(subBladeStart, _("The starting pixel number for the current Sub-Blade.\nThis number starts at 0."));
    TIP(subBladeEnd, _("The ending pixel number for the current Sub-Blade.\nThis number should not exceed the \"Number of Pixels\" in the blade."));


    TIP(star1Color, _("The profile/type of the first LED.\nCorresponds to the first-selected power pin."));
    TIP(star2Color, _("The profile/type of the second LED.\nCorresponds to the second-selected power pin."));
    TIP(star3Color, _("The profile/type of the third LED.\nCorresponds to the third-selected power pin."));
    TIP(star4Color, _("The profile/type of the fourth LED.\nCorresponds to the fourth-selected power pin."));
    TIP(star1Resistance, _("The value of the resistor placed in series with this led."));
    TIP(star2Resistance, _("The value of the resistor placed in series with this led."));
    TIP(star3Resistance, _("The value of the resistor placed in series with this led."));
    TIP(star4Resistance, _("The value of the resistor placed in series with this led."));
}

wxBoxSizer *BladesPage::createBladeSelect() {
    auto *bladeSelectSizer{new wxBoxSizer(wxVERTICAL)};
    bladeArray = new PCUI::Choice(GetStaticBox(), ID_BladeArray,  Misc::createEntries({ "blade_in" }), _("Blade Array"));
    bladeArrayButton = new wxButton(GetStaticBox(), ID_OpenBladeArrays, _("Blade Awareness..."));
    bladeSelectSizer->Add(bladeArray, TEXTITEMFLAGS.Expand());
    bladeSelectSizer->Add(bladeArrayButton, wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 5).Expand());
    bladeSelectSizer->Add(createBladeManager(), wxSizerFlags(1).Border(wxALL, 5).Expand());

    return bladeSelectSizer;
}

wxBoxSizer *BladesPage::createBladeManager() {
    auto *bladeManagerSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *bladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    auto *bladeText{new wxStaticText(GetStaticBox(), wxID_ANY, _("Blades"))};
    bladeSelect = new wxListBox(GetStaticBox(), ID_BladeSelect, wxDefaultPosition, wxDefaultSize, wxArrayString{}, wxNO_BORDER);
    auto *bladeButtons{new wxBoxSizer(wxHORIZONTAL)};
    addBladeButton = new wxButton(GetStaticBox(), ID_AddBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    removeBladeButton = new wxButton(GetStaticBox(), ID_RemoveBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    bladeButtons->Add(addBladeButton, wxSizerFlags(0).Border(wxRIGHT, 10));
    bladeButtons->Add(removeBladeButton, wxSizerFlags(0));
    bladeSelectionSizer->Add(bladeText, wxSizerFlags(0));
    bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand().Border(wxBOTTOM, 5));
    bladeSelectionSizer->Add(bladeButtons, wxSizerFlags(0).Center());

    auto *subBladeSelectionSizer{new wxBoxSizer(wxVERTICAL)};
    auto *subBladeText{new wxStaticText(GetStaticBox(), wxID_ANY, _("SubBlades"))};
    subBladeSelect = new wxListBox(GetStaticBox(), ID_SubBladeSelect, wxDefaultPosition, wxSize(100, -1), wxArrayString{}, wxNO_BORDER);
    auto *subBladeButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    addSubBladeButton = new wxButton(GetStaticBox(), ID_AddSubBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    removeSubBladeButton = new wxButton(GetStaticBox(), ID_RemoveSubBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    subBladeButtonSizer->Add(addSubBladeButton, wxSizerFlags(0).Border(wxRIGHT, 10));
    subBladeButtonSizer->Add(removeSubBladeButton, wxSizerFlags(0));
    subBladeSelectionSizer->Add(subBladeText, wxSizerFlags(0));
    subBladeSelectionSizer->Add(subBladeSelect, wxSizerFlags(1).Expand().Border(wxBOTTOM, 5));
    subBladeSelectionSizer->Add(subBladeButtonSizer, wxSizerFlags(0).Center());

    bladeManagerSizer->Add(bladeSelectionSizer, wxSizerFlags(1).Expand());
    bladeManagerSizer->Add(subBladeSelectionSizer, wxSizerFlags(1).Expand());

    return bladeManagerSizer;
}

wxBoxSizer *BladesPage::createBladeSetup() {
    auto *bladeSetup{new wxBoxSizer(wxVERTICAL)};
    bladeType = new PCUI::Choice(GetStaticBox(), ID_BladeType,  Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_SIMPLE}), _("Blade Type"));
    powerPins = new wxCheckListBox(GetStaticBox(), ID_PowerPins, wxDefaultPosition, wxSize(200, -1), Misc::createEntries({"bladePowerPin1", "bladePowerPin2", "bladePowerPin3", "bladePowerPin4", "bladePowerPin5", "bladePowerPin6"}), wxBORDER_NONE);
    auto *pinNameSizer{new wxBoxSizer(wxHORIZONTAL)};
    addPowerPin = new wxButton(GetStaticBox(), ID_AddPowerPin, "+", wxDefaultPosition, wxSize(30, 20), wxBU_EXACTFIT);
    powerPinName = new PCUI::Text(GetStaticBox(), ID_PowerPinName, {}, 0, _("Pin Name"));
    pinNameSizer->Add(powerPinName, wxSizerFlags(1).Border(wxRIGHT, 5));
    pinNameSizer->Add(addPowerPin, wxSizerFlags(0).Bottom());

    bladeSetup->Add(bladeType, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());
    bladeSetup->Add(powerPins, wxSizerFlags(1).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());
    bladeSetup->Add(pinNameSizer, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());

    return bladeSetup;
}

wxBoxSizer *BladesPage::createBladeSettings() {
    auto *bladeSettings{new wxBoxSizer(wxVERTICAL)};
    auto *bladeColor{new wxBoxSizer(wxVERTICAL)};
    blade3ColorOrder = new PCUI::Choice(GetStaticBox(), wxID_ANY, Misc::createEntries({"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}), _("Color Order"));
    blade4ColorOrder = new PCUI::Choice(GetStaticBox(), wxID_ANY, Misc::createEntries({"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}), _("Color Order"));
    bladeColor->Add(blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeColor->Add(blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    blade4UseRGB = new wxCheckBox(GetStaticBox(), wxID_ANY, _("Use RGB with White"));
    bladeDataPin = new PCUI::ComboBox(GetStaticBox(), wxID_ANY, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}), {}, _("Blade Data Pin"));
    bladePixels = new PCUI::Numeric(GetStaticBox(), wxID_ANY, 0, std::numeric_limits<int32>::max(), 144, 1, wxSP_ARROW_KEYS, _("Number of Pixels"));

    star1Sizer = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), _("LED 1"));
    star1Color = new PCUI::Choice(star1Sizer->GetStaticBox(), ID_LEDColor, ledStrings());
    star1Resistance = new PCUI::Numeric(star1Sizer->GetStaticBox(), wxID_ANY, 0, 10000, 500, 1, wxSP_ARROW_KEYS, _("Resistance (mOhms)"));
    star1Sizer->Add(star1Color, MENUITEMFLAGS);
    star1Sizer->Add(star1Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).DoubleBorder(wxBOTTOM).Expand());

    star2Sizer = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), _("LED 2"));
    star2Color = new PCUI::Choice(star2Sizer->GetStaticBox(), ID_LEDColor, ledStrings());
    star2Resistance = new PCUI::Numeric(star2Sizer->GetStaticBox(), wxID_ANY, 0, 10000, 500, 1, wxSP_ARROW_KEYS, _("Resistance (mOhms)"));
    star2Sizer->Add(star2Color, MENUITEMFLAGS);
    star2Sizer->Add(star2Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).DoubleBorder(wxBOTTOM).Expand());

    star3Sizer = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), _("LED 3"));
    star3Color = new PCUI::Choice(star3Sizer->GetStaticBox(), ID_LEDColor, ledStrings());
    star3Resistance = new PCUI::Numeric(star3Sizer->GetStaticBox(), wxID_ANY, 0, 10000, 500, 1, wxSP_ARROW_KEYS, _("Resistance (mOhms)"));
    star3Sizer->Add(star3Color, MENUITEMFLAGS);
    star3Sizer->Add(star3Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).DoubleBorder(wxBOTTOM).Expand());

    star4Sizer = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), _("LED 4"));
    star4Color = new PCUI::Choice(star4Sizer->GetStaticBox(), ID_LEDColor, ledStrings());
    star4Resistance = new PCUI::Numeric(star4Sizer->GetStaticBox(), wxID_ANY, 0, 10000, 500, 1, wxSP_ARROW_KEYS, _("Resistance (mOhms)"));
    star4Sizer->Add(star4Color, MENUITEMFLAGS);
    star4Sizer->Add(star4Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).DoubleBorder(wxBOTTOM).Expand());

    useStandard = new wxRadioButton(GetStaticBox(), wxID_ANY, _("Standard SubBlade"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    useStride   = new wxRadioButton(GetStaticBox(), wxID_ANY, _("Stride SubBlade"));
    useZigZag   = new wxRadioButton(GetStaticBox(), wxID_ANY, _("ZigZag SubBlade"));
    subBladeStart = new PCUI::Numeric(GetStaticBox(), wxID_ANY, 0, std::numeric_limits<int32>::max(), 0, 1, wxSP_ARROW_KEYS, _("SubBlade Start"));
    subBladeEnd = new PCUI::Numeric(GetStaticBox(), wxID_ANY, 0, std::numeric_limits<int32>::max(), 0, 1, wxSP_ARROW_KEYS, _("SubBlade End"));

    bladeSettings->Add(bladeColor);
    bladeSettings->Add(blade4UseRGB, MENUITEMFLAGS);
    bladeSettings->Add(star1Sizer, MENUITEMFLAGS);
    bladeSettings->Add(star2Sizer, MENUITEMFLAGS);
    bladeSettings->Add(star3Sizer, MENUITEMFLAGS);
    bladeSettings->Add(star4Sizer, MENUITEMFLAGS);
    bladeSettings->Add(bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(bladePixels, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    bladeSettings->Add(useStandard, MENUITEMFLAGS);
    bladeSettings->Add(useStride, MENUITEMFLAGS);
    bladeSettings->Add(useZigZag, MENUITEMFLAGS);
    bladeSettings->Add(subBladeStart, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(subBladeEnd, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    bladeSettings->SetMinSize(150, -1);
    return bladeSettings;
}

void BladesPage::update() {
    saveCurrent();
    rebuildBladeArray();
    updateRanges();
    loadSettings();
    setEnabled();
    setVisibility();

    mParent->Layout();
    mParent->GetSizer()->Fit(mParent);
}

void BladesPage::saveCurrent() const {
    if (
            lastBladeArraySelection < 0 ||
            lastBladeArraySelection > (int32_t)bladeArrayDlg->bladeArrays.size() ||
            mLastBladeSelection < 0 ||
            mLastBladeSelection >= (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size()
       ) {
        return;
    }

    auto& lastBlade = bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(mLastBladeSelection);
    lastBlade.type = bladeType->entry()->GetStringSelection();
    lastBlade.powerPins.clear();
    for (uint32_t idx = 0; idx < powerPins->GetCount(); idx++) {
        if (powerPins->IsChecked(idx)) lastBlade.powerPins.emplace_back(powerPins->GetString(idx).ToStdString());
    }

    lastBlade.dataPin = bladeDataPin->entry()->GetStringSelection();
    lastBlade.numPixels = bladePixels->entry()->GetValue();
    lastBlade.colorType = lastBlade.type == BD_PIXELRGB ? blade3ColorOrder->entry()->GetStringSelection() : blade4ColorOrder->entry()->GetStringSelection();
    lastBlade.useRGBWithWhite = blade4UseRGB->GetValue();

    lastBlade.star1 = strToLed(star1Color->entry()->GetStringSelection());
    lastBlade.star1Resistance = star1Resistance->entry()->GetValue();
    lastBlade.star2 = strToLed(star2Color->entry()->GetStringSelection());
    lastBlade.star2Resistance = star2Resistance->entry()->GetValue();
    lastBlade.star3 = strToLed(star3Color->entry()->GetStringSelection());
    lastBlade.star3Resistance = star3Resistance->entry()->GetValue();
    lastBlade.star4 = strToLed(star4Color->entry()->GetStringSelection());
    lastBlade.star4Resistance = star4Resistance->entry()->GetValue();

    if (mLastSubBladeSelection != -1 && mLastSubBladeSelection < (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.size()) {
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.at(mLastSubBladeSelection).startPixel = subBladeStart->entry()->GetValue();
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.at(mLastSubBladeSelection).endPixel = subBladeEnd->entry()->GetValue();
    }
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).useStride = useStride->GetValue();
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).useZigZag = useZigZag->GetValue();

    // Check if SubBlades need to be removed (changed from WX281X)
    if (hasBladeSelection() and mLastBladeSelection == bladeSelect->GetSelection() and not isPixelBlade()) {
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).isSubBlade = false;
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.clear();
    }
}

void BladesPage::rebuildBladeArray() {
    if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() == 0) {
        BladeConfig bladeConfig{
            .numPixels = 144,
            .powerPins{ "bladePowerPin2", "bladePowerPin3"}
        };
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.emplace_back(bladeConfig);
    }

    mLastBladeSelection = bladeSelect->GetSelection();
    mLastSubBladeSelection = subBladeSelect->GetSelection();
    lastBladeArraySelection = bladeArray->entry()->GetSelection();

    bladeArray->entry()->Clear();
    for (BladeArrayDlg::BladeArray& array : bladeArrayDlg->bladeArrays) {
        bladeArray->entry()->Append(array.name);
    }
    if (lastBladeArraySelection >= 0 && lastBladeArraySelection < static_cast<int32_t>(bladeArray->entry()->GetCount())) {
        bladeArray->entry()->SetSelection(lastBladeArraySelection);
    } else bladeArray->entry()->SetSelection(0);

    bladeSelect->Clear();
    for (auto bladeNum{0}; bladeNum < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size()); ++bladeNum) {
        bladeSelect->Append(wxString::Format(_("Blade %d"), std::to_string(bladeNum)));
    }

    if (mLastBladeSelection >= 0 && mLastBladeSelection < static_cast<int32_t>(bladeSelect->GetCount())) {
        bladeSelect->SetSelection(mLastBladeSelection);
    }

    subBladeSelect->Clear();
    if (
            bladeSelect->GetSelection() >= 0 and 
            bladeSelect->GetSelection() < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size())
       ) {
        for (auto subBladeNum{0}; bladeSelect->GetSelection() != -1 and subBladeNum < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size()); ++subBladeNum) {
            subBladeSelect->Append(wxString::Format(_("SubBlade %d"), std::to_string(subBladeNum)));
        }

        if (mLastSubBladeSelection >= 0 && mLastSubBladeSelection < static_cast<int32_t>(subBladeSelect->GetCount())) subBladeSelect->SetSelection(mLastSubBladeSelection);
    }
}

void BladesPage::loadSettings() const {
    const auto& blades{bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades};

    if (bladeSelect->GetSelection() < 0 || bladeSelect->GetSelection() >= (int32_t)blades.size()) return;

    const auto& selectedBlade{blades.at(bladeSelect->GetSelection())};

    bladeType->entry()->SetStringSelection(selectedBlade.type);
    for (auto idx{0}; idx < powerPins->GetCount(); ++idx) {
        powerPins->Check(idx, false);
    }
    for (const auto& powerPin : selectedBlade.powerPins) {
        powerPins->Check(powerPins->FindString(powerPin));
    }

    bladeDataPin->entry()->SetStringSelection(selectedBlade.dataPin);
    bladePixels->entry()->SetValue(selectedBlade.numPixels);
    blade3ColorOrder->entry()->SetStringSelection(selectedBlade.colorType);
    blade4ColorOrder->entry()->SetStringSelection(selectedBlade.colorType);
    blade4UseRGB->SetValue(selectedBlade.useRGBWithWhite);

    star1Color->entry()->SetStringSelection(ledToStr(selectedBlade.star1));
    star1Resistance->entry()->SetValue(selectedBlade.star1Resistance);
    star2Color->entry()->SetStringSelection(ledToStr(selectedBlade.star2));
    star2Resistance->entry()->SetValue(selectedBlade.star2Resistance);
    star3Color->entry()->SetStringSelection(ledToStr(selectedBlade.star3));
    star3Resistance->entry()->SetValue(selectedBlade.star3Resistance);
    star4Color->entry()->SetStringSelection(ledToStr(selectedBlade.star4));
    star4Resistance->entry()->SetValue(selectedBlade.star4Resistance);

    subBladeStart->entry()->SetValue(mLastSubBladeSelection != -1 and mLastSubBladeSelection < (int32_t)selectedBlade.subBlades.size() ? selectedBlade.subBlades.at(mLastSubBladeSelection).startPixel : 0);
    subBladeEnd->entry()->SetValue(mLastSubBladeSelection != -1 and mLastSubBladeSelection < (int32_t)selectedBlade.subBlades.size() ? selectedBlade.subBlades.at(mLastSubBladeSelection).endPixel : 0);
    useStandard->SetValue(not selectedBlade.useStride and not selectedBlade.useZigZag);
    useStride->SetValue(selectedBlade.useStride);
    useZigZag->SetValue(selectedBlade.useZigZag);
}

void BladesPage::setEnabled() {
    removeBladeButton->Enable(bladeSelect->GetCount() > 1 and hasBladeSelection());
    removeSubBladeButton->Enable(subBladeSelect->GetCount() > 0 and hasSubBladeSelection());
    addSubBladeButton->Enable(isPixelBlade() and hasBladeSelection());

    bladeType->entry()->Enable(hasBladeSelection() and isFirstBlade());
    powerPins->Enable(hasBladeSelection() and isFirstBlade());
    addPowerPin->Enable(hasBladeSelection() and isFirstBlade() and not powerPinName->entry()->IsEmpty());
    powerPinName->entry()->Enable(hasBladeSelection() and isFirstBlade());

    star1Resistance->entry()->Enable(CREE_LED & strToLed(star1Color->entry()->GetStringSelection()));
    star2Resistance->entry()->Enable(CREE_LED & strToLed(star2Color->entry()->GetStringSelection()));
    star3Resistance->entry()->Enable(CREE_LED & strToLed(star3Color->entry()->GetStringSelection()));
    star4Resistance->entry()->Enable(CREE_LED & strToLed(star4Color->entry()->GetStringSelection()));
}

void BladesPage::setVisibility(){
    blade3ColorOrder->Show(isRGBPixelBlade() and isFirstBlade());
    blade4ColorOrder->Show(isRGBWPixelBlade() and isFirstBlade());
    blade4UseRGB->Show(isRGBWPixelBlade() and isFirstBlade());

    bladeDataPin->Show(isPixelBlade() and isFirstBlade());
    bladePixels->Show(isPixelBlade() and isFirstBlade());

    star1Sizer->Show(isSimpleBlade());
    star2Sizer->Show(isSimpleBlade());
    star3Sizer->Show(isSimpleBlade());
    star4Sizer->Show(isSimpleBlade());

    useStandard->Show(isSubBlade() and isFirstBlade());
    useStride->Show(isSubBlade() and isFirstBlade());
    useZigZag->Show(isSubBlade() and isFirstBlade());
    subBladeStart->Show(hasSubBladeSelection() and isStandardSubBlade());
    subBladeEnd->Show(hasSubBladeSelection() and isStandardSubBlade());
}

void BladesPage::updateRanges() {
    const auto& blades = bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).blades;
    bladePixels->entry()->SetRange(
        bladeSelect->GetSelection() >= 0 ? 
            static_cast<int32>(blades.at(bladeSelect->GetSelection()).subBlades.size()) : 
            1, 
        mParent->generalPage->maxLEDs->entry()->GetValue()
    );

    if (subBladeSelect ->GetSelection() >= 0 && blades.at(bladeSelect->GetSelection()).isSubBlade) {
        subBladeStart->entry()->SetRange(0, blades.at(bladeSelect->GetSelection()).numPixels - 1);
        subBladeEnd->entry()->SetRange(blades.at(bladeSelect->GetSelection()).subBlades.at(subBladeSelect->GetSelection()).startPixel, blades.at(bladeSelect->GetSelection()).numPixels - 1);
    }
}

void BladesPage::addBlade() {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.emplace_back();
    update();
}

void BladesPage::addSubBlade() {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).isSubBlade = true;
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.emplace_back();
    if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.size() <= 1) {
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.emplace_back();
    }
    update();
}

void BladesPage::removeBlade() {
    saveCurrent();

    if (hasBladeSelection() and bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 1) {
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.erase(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.begin() + mLastBladeSelection);
    }

    update();
}

void BladesPage::removeSubBlade() {
    saveCurrent();

    if (hasSubBladeSelection()) {
        bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.erase(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.begin() + mLastSubBladeSelection);
        if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.size() <= 1) {
            bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).subBlades.clear();
            bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(mLastBladeSelection).isSubBlade = false;
        }
        mLastSubBladeSelection = -1;
    }

    update();
}
