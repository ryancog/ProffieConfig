// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/pages/bladespage.h"

#include "editor/editorwindow.h"
#include "editor/pages/generalpage.h"
#include "editor/dialogs/bladearraydlg.h"
#include "core/utilities/misc.h"
#include "core/defines.h"
#include "wx/gdicmn.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/tooltip.h>

BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, ""), parent{static_cast<EditorWindow*>(window)} {
  bladeArrayDlg = new BladeArrayDlg(parent);

  Add(createBladeSelect(), wxSizerFlags(0).Expand());
  Add(createBladeSetup(), wxSizerFlags(1).Expand());
  Add(createBladeSettings(), wxSizerFlags(0).ReserveSpaceEvenIfHidden());

  bindEvents();
  createToolTips();
}

void BladesPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { if (bladeArrayDlg->IsShown()) bladeArrayDlg->Raise(); else bladeArrayDlg->Show(); }, ID_OpenBladeArrays);

  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { parent->presetsPage->bladeArray->entry()->SetSelection(bladeArray->entry()->GetSelection()); update(); }, ID_BladeArray);
  GetStaticBox()->Bind(wxEVT_SPINCTRL, [&](wxCommandEvent& event) { update(); event.Skip(); });
  GetStaticBox()->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent& event) { update(); event.Skip(); });
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { update(); FULLUPDATEWINDOW(parent); }, ID_BladeSelect);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { update(); FULLUPDATEWINDOW(parent); }, ID_SubBladeSelect);
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { update(); FULLUPDATEWINDOW(parent); }, ID_BladeType);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addBlade(); UPDATEWINDOW(parent); }, ID_AddBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addSubBlade(); FULLUPDATEWINDOW(parent); }, ID_AddSubBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeBlade(); UPDATEWINDOW(parent); }, ID_RemoveBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeSubBlade(); UPDATEWINDOW(parent); }, ID_RemoveSubBlade);
  GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
      size_t insertionPoint = powerPinName->entry()->GetInsertionPoint();
      std::string newString{};
      for (char c : powerPinName->entry()->GetValue().ToStdString()) if (isalnum(c)) newString += c;

      powerPinName->entry()->ChangeValue(newString);
      powerPinName->entry()->SetInsertionPoint(insertionPoint < newString.length() ? insertionPoint : newString.length());
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
void BladesPage::createToolTips() {
  TIP(bladeArray, "The currently-selected Blade Array to edit.");
  TIP(addBladeButton, "Add a blade to the selected blade array.");
  TIP(removeBladeButton, "Remove the currently-selected blade.");
  TIP(addSubBladeButton, "Add a Sub-Blade to the currently-selected blade.\nCan only be used with WS281X-type blades.");
  TIP(removeSubBladeButton, "Remove the currently-selected Sub-Blade.\nIf there are less than 2 Sub-Blades after removal, the remaining Sub-Blade will be deleted.");

  TIP(bladeType, "The type of blade/LED.");
  TIP(powerPins, "The power pins to use for this blade.\nWS281X blades can have as many as are desired (though 2 is generally enough for most blades)");
  TIP(blade3ColorOrder, "The order of colors for your blade.\nMost of the time this can be left as \"GRB\".");
  TIP(blade4ColorOrder, "The order of colors for your blade.\nMost of the time this can be left as \"GRBW\".");
  TIP(blade4UseRGB, "Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white.");
  TIP(bladeDataPin, "The pin name or number used for WS281X data.\nSpecify custom pins by typing in this box.");
  TIP(bladePixels, "The number of pixels in your blade (total).");

  TIP(useStandard, "Split apart data into continuous given sections.");
  TIP(useStride, "Useful to KR style blades and other similar types where the data signal \"strides\" back and forth across sides.");
  TIP(useZigZag, "Similar to using stride, but for blades in which the data is continuous, \"zig-zagging\" up and down the blade.");
  TIP(subBladeStart, "The starting pixel number for the current Sub-Blade.\nThis number starts at 0.");
  TIP(subBladeStart, "The ending pixel number for the current Sub-Blade.\nThis number should not exceed the \"Number of Pixels\" in the blade.");


  TIP(star1Color, "The color of the first LED on the star.\nCorresponds to the first-selected power pin.");
  TIP(star2Color, "The color of the second LED on the star.\nCorresponds to the second-selected power pin.");
  TIP(star3Color, "The color of the third LED on the star.\nCorresponds to the third-selected power pin.");
  TIP(star4Color, "The color of the fourth LED on the star.\nCorresponds to the fourth-selected power pin.");
  TIP(star1Resistance, "The value of the resistor placed in series with this star.");
  TIP(star2Resistance, "The value of the resistor placed in series with this star.");
  TIP(star3Resistance, "The value of the resistor placed in series with this star.");
  TIP(star4Resistance, "The value of the resistor placed in series with this star.");
}

wxBoxSizer* BladesPage::createBladeSelect() {
  wxBoxSizer* bladeSelectSizer = new wxBoxSizer(wxVERTICAL);
  bladeArray = new pcComboBox(GetStaticBox(), ID_BladeArray, "Blade Array", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ "blade_in" }), wxCB_READONLY);
  bladeArrayButton = new wxButton(GetStaticBox(), ID_OpenBladeArrays, "Blade Awareness...");
  bladeSelectSizer->Add(bladeArray, TEXTITEMFLAGS.Expand());
  bladeSelectSizer->Add(bladeArrayButton, wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 5).Expand());
  bladeSelectSizer->Add(createBladeManager(), wxSizerFlags(1).Border(wxALL, 5).Expand());

  return bladeSelectSizer;
}
wxBoxSizer* BladesPage::createBladeManager() {
  wxBoxSizer* bladeManagerSizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* bladeSelectionSizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* bladeText = new wxStaticText(GetStaticBox(), wxID_ANY, "Blades");
  bladeSelect = new wxListBox(GetStaticBox(), ID_BladeSelect);
  wxBoxSizer* bladeButtons = new wxBoxSizer(wxHORIZONTAL);
  addBladeButton = new wxButton(GetStaticBox(), ID_AddBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeBladeButton = new wxButton(GetStaticBox(), ID_RemoveBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  bladeButtons->Add(addBladeButton, wxSizerFlags(0).Border(wxRIGHT, 10));
  bladeButtons->Add(removeBladeButton, wxSizerFlags(0));
  bladeSelectionSizer->Add(bladeText, wxSizerFlags(0));
  bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand().Border(wxBOTTOM, 5));
  bladeSelectionSizer->Add(bladeButtons, wxSizerFlags(0).Center());

  wxBoxSizer* subBladeSelectionSizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* subBladeText = new wxStaticText(GetStaticBox(), wxID_ANY, "SubBlades");
  subBladeSelect = new wxListBox(GetStaticBox(), ID_SubBladeSelect, wxDefaultPosition, wxSize(100, -1));
  wxBoxSizer* subBladeButtonSizer = new wxBoxSizer(wxHORIZONTAL);
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
wxBoxSizer* BladesPage::createBladeSetup() {
  wxBoxSizer* bladeSetup = new wxBoxSizer(wxVERTICAL);
  bladeType = new pcComboBox(GetStaticBox(), ID_BladeType, "Blade Type", wxDefaultPosition, wxDefaultSize, Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_TRISTAR, BD_QUADSTAR, BD_SINGLELED}), wxCB_READONLY);
  powerPins = new wxCheckListBox(GetStaticBox(), ID_PowerPins, wxDefaultPosition, wxSize(200, -1), Misc::createEntries({"bladePowerPin1", "bladePowerPin2", "bladePowerPin3", "bladePowerPin4", "bladePowerPin5", "bladePowerPin6"}));
  auto pinNameSizer = new wxBoxSizer(wxHORIZONTAL);
  addPowerPin = new wxButton(GetStaticBox(), ID_AddPowerPin, "+", wxDefaultPosition, wxSize(30, 20), wxBU_EXACTFIT);
  powerPinName = new pcTextCtrl(GetStaticBox(), ID_PowerPinName, "Pin Name");
  pinNameSizer->Add(powerPinName, wxSizerFlags(1).Border(wxRIGHT, 5));
  pinNameSizer->Add(addPowerPin, wxSizerFlags(0).Bottom());

  bladeSetup->Add(bladeType, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());
  bladeSetup->Add(powerPins, wxSizerFlags(1).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());
  bladeSetup->Add(pinNameSizer, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5).Expand());

  return bladeSetup;
}
wxBoxSizer* BladesPage::createBladeSettings() {
  wxWrapSizer* bladeSettings = new wxWrapSizer(wxVERTICAL);
  wxBoxSizer* bladeColor = new wxBoxSizer(wxVERTICAL);
  blade3ColorOrder = new pcComboBox(GetStaticBox(), wxID_ANY, "Color Order", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}), wxCB_READONLY);
  blade4ColorOrder = new pcComboBox(GetStaticBox(), wxID_ANY, "Color Order", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}), wxCB_READONLY);
  bladeColor->Add(blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeColor->Add(blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  blade4UseRGB = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use RGB with White");
  bladeDataPin = new pcComboBox(GetStaticBox(), wxID_ANY, "Blade Data Pin", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}));
  bladePixelsLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Number of Pixels");
  bladePixels = new pcSpinCtrl(GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

  wxBoxSizer* star1 = new wxBoxSizer(wxVERTICAL);
  star1Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 1 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star1Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star1->Add(star1Color, MENUITEMFLAGS);
  star1->Add(star1Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).Expand());

  wxBoxSizer* star2 = new wxBoxSizer(wxVERTICAL);
  star2Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 2 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star2Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star2->Add(star2Color, MENUITEMFLAGS);
  star2->Add(star2Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).Expand());

  wxBoxSizer* star3 = new wxBoxSizer(wxVERTICAL);
  star3Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 3 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star3Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star3->Add(star3Color, MENUITEMFLAGS);
  star3->Add(star3Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).Expand());

  wxBoxSizer* star4 = new wxBoxSizer(wxVERTICAL);
  star4Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 4 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star4Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star4->Add(star4Color, MENUITEMFLAGS);
  star4->Add(star4Resistance, MENUITEMFLAGS.TripleBorder(wxLEFT).Expand());

  useStandard = new wxRadioButton(GetStaticBox(), wxID_ANY, "Standard SubBlade", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  useStride   = new wxRadioButton(GetStaticBox(), wxID_ANY, "Stride SubBlade");
  useZigZag   = new wxRadioButton(GetStaticBox(), wxID_ANY, "ZigZag SubBlade");
  subBladeStart = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "SubBlade Start", wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
  subBladeEnd   = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "SubBlade End", wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

  bladeSettings->Add(bladeColor);
  bladeSettings->Add(blade4UseRGB, MENUITEMFLAGS);
  bladeSettings->Add(star1, MENUITEMFLAGS);
  bladeSettings->Add(star2, MENUITEMFLAGS);
  bladeSettings->Add(star3, MENUITEMFLAGS);
  bladeSettings->Add(star4, MENUITEMFLAGS);
  bladeSettings->Add(bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(bladePixelsLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
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
}

void BladesPage::saveCurrent() {
  if (lastBladeArraySelection < 0 ||
    lastBladeArraySelection > (int32_t)bladeArrayDlg->bladeArrays.size() ||
    lastBladeSelection < 0 ||
    lastBladeSelection >= (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size()
    ) {
    return;
  }

  auto& lastBlade = bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection);
  lastBlade.type = bladeType->entry()->GetValue();
  lastBlade.powerPins.clear();
  for (uint32_t idx = 0; idx < powerPins->GetCount(); idx++) {
    if (powerPins->IsChecked(idx)) lastBlade.powerPins.push_back(powerPins->GetString(idx).ToStdString());
  }
  
  lastBlade.dataPin = bladeDataPin->entry()->GetValue();
  lastBlade.numPixels = bladePixels->entry()->GetValue();
  lastBlade.colorType = lastBlade.type == BD_PIXELRGB ? blade3ColorOrder->entry()->GetValue() : blade4ColorOrder->entry()->GetValue();
  lastBlade.useRGBWithWhite = blade4UseRGB->GetValue();
  
  lastBlade.Star1 = star1Color->entry()->GetValue();
  lastBlade.Star1Resistance = star1Resistance->entry()->GetValue();
  lastBlade.Star2 = star2Color->entry()->GetValue();
  lastBlade.Star2Resistance = star2Resistance->entry()->GetValue();
  lastBlade.Star3 = star3Color->entry()->GetValue();
  lastBlade.Star3Resistance = star3Resistance->entry()->GetValue();
  lastBlade.Star4 = star4Color->entry()->GetValue();
  lastBlade.Star4Resistance = star4Resistance->entry()->GetValue();
  
  if (lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.size()) {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).startPixel = subBladeStart->entry()->GetValue();
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).endPixel = subBladeEnd->entry()->GetValue();
  }
  bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).useStride = useStride->GetValue();
  bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).useZigZag = useZigZag->GetValue();

         // Check if SubBlades need to be removed (changed from WX281X)
  if (BD_HASSELECTION && lastBladeSelection == bladeSelect->GetSelection() && !BD_ISPIXEL) {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).isSubBlade = false;
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.clear();
  }
}
void BladesPage::rebuildBladeArray() {
  if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() == 0) bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.push_back(BladeConfig({ .numPixels = 144, .powerPins{ "bladePowerPin2", "bladePowerPin3"} }));

  lastBladeSelection = bladeSelect->GetSelection();
  lastSubBladeSelection = subBladeSelect->GetSelection();
  lastBladeArraySelection = bladeArray->entry()->GetSelection();

  bladeArray->entry()->Clear();
  for (BladeArrayDlg::BladeArray& array : bladeArrayDlg->bladeArrays) {
    bladeArray->entry()->Append(array.name);
  }
  if (lastBladeArraySelection >= 0 && lastBladeArraySelection < static_cast<int32_t>(bladeArray->entry()->GetCount())) bladeArray->entry()->SetSelection(lastBladeArraySelection);
  else bladeArray->entry()->SetSelection(0);

  bladeSelect->Clear();
  for (int32_t bladeNum = 0; bladeNum < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size()); bladeNum++)
    bladeSelect->Append("Blade " + std::to_string(bladeNum));
  if (lastBladeSelection >= 0 && lastBladeSelection < static_cast<int32_t>(bladeSelect->GetCount())) bladeSelect->SetSelection(lastBladeSelection);

  subBladeSelect->Clear();
  if (bladeSelect->GetSelection() >= 0 && bladeSelect->GetSelection() < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size())) {
    for (int32_t subBladeNum = 0; bladeSelect->GetSelection() != -1 && subBladeNum < static_cast<int32_t>(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size()); subBladeNum++)
      subBladeSelect->Append("SubBlade " + std::to_string(subBladeNum));
    if (lastSubBladeSelection >= 0 && lastSubBladeSelection < static_cast<int32_t>(subBladeSelect->GetCount())) subBladeSelect->SetSelection(lastSubBladeSelection);
  }
}
void BladesPage::loadSettings() {
  const auto& blades = bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades;

  if (bladeSelect->GetSelection() < 0 || bladeSelect->GetSelection() >= (int32_t)blades.size()) return;

  const auto& selectedBlade = blades.at(bladeSelect->GetSelection());

  bladeType->entry()->SetValue(selectedBlade.type);
  for (uint32_t idx = 0; idx < powerPins->GetCount(); idx++) {
    powerPins->Check(idx, false);
  }
  for (int32_t idx = 0; idx < static_cast<int32_t>(selectedBlade.powerPins.size()); idx++) {
    powerPins->Check(powerPins->FindString(selectedBlade.powerPins.at(idx)));
  }

  bladeDataPin->entry()->SetValue(selectedBlade.dataPin);
  bladePixels->entry()->SetValue(selectedBlade.numPixels);
  blade3ColorOrder->entry()->SetValue(selectedBlade.colorType);
  blade4ColorOrder->entry()->SetValue(selectedBlade.colorType);
  blade4UseRGB->SetValue(selectedBlade.useRGBWithWhite);
  
  star1Color->entry()->SetValue(selectedBlade.Star1);
  star1Resistance->entry()->SetValue(selectedBlade.Star1Resistance);
  star2Color->entry()->SetValue(selectedBlade.Star2);
  star2Resistance->entry()->SetValue(selectedBlade.Star2Resistance);
  star3Color->entry()->SetValue(selectedBlade.Star3);
  star3Resistance->entry()->SetValue(selectedBlade.Star3Resistance);
  star4Color->entry()->SetValue(selectedBlade.Star4);
  star4Resistance->entry()->SetValue(selectedBlade.Star4Resistance);
  
  subBladeStart->entry()->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)selectedBlade.subBlades.size() ? selectedBlade.subBlades.at(lastSubBladeSelection).startPixel : 0);
  subBladeEnd->entry()->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)selectedBlade.subBlades.size() ? selectedBlade.subBlades.at(lastSubBladeSelection).endPixel : 0);
  useStride->SetValue(selectedBlade.useStride);
  useZigZag->SetValue(selectedBlade.useZigZag);
}
void BladesPage::setEnabled() {
  removeBladeButton->Enable(bladeSelect->GetCount() > 1 && BD_HASSELECTION);
  removeSubBladeButton->Enable(subBladeSelect->GetCount() > 0 && BD_SUBHASSELECTION);
  addSubBladeButton->Enable(BD_ISPIXEL && BD_HASSELECTION);

  bladeType->Enable(BD_HASSELECTION && BD_ISFIRST);
  powerPins->Enable(BD_HASSELECTION && BD_ISFIRST);
  addPowerPin->Enable(BD_HASSELECTION && BD_ISFIRST && !powerPinName->entry()->IsEmpty());
  powerPinName->Enable(BD_HASSELECTION && BD_ISFIRST);

  star1Resistance->Enable(star1Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star2Resistance->Enable(star2Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star3Resistance->Enable(star3Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star4Resistance->Enable(star4Color->entry()->GetStringSelection() != BD_NORESISTANCE);
}
void BladesPage::setVisibility(){
  blade3ColorOrder->Show(BD_ISPIXEL3 && BD_ISFIRST);
  blade4ColorOrder->Show(BD_ISPIXEL4 && BD_ISFIRST);
  blade4UseRGB->Show(BD_ISPIXEL4 && BD_ISFIRST);

  bladeDataPin->Show(BD_ISPIXEL && BD_ISFIRST);
  bladePixelsLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  bladePixels->Show(BD_ISPIXEL && BD_ISFIRST);

  star1Color->Show(BD_ISSTAR);
  star1Resistance->Show(BD_ISSTAR);
  star2Color->Show(BD_ISSTAR);
  star2Resistance->Show(BD_ISSTAR);
  star3Color->Show(BD_ISSTAR);
  star3Resistance->Show(BD_ISSTAR);
  star4Color->Show(BD_ISSTAR4);
  star4Resistance->Show(BD_ISSTAR4);

  useStandard->Show(BD_ISSUB && BD_ISFIRST);
  useStride->Show(BD_ISSUB && BD_ISFIRST);
  useZigZag->Show(BD_ISSUB && BD_ISFIRST);
  subBladeStart->Show(BD_SUBHASSELECTION && BD_ISSTNDRD);
  subBladeEnd->Show(BD_SUBHASSELECTION && BD_ISSTNDRD);
}
void BladesPage::updateRanges() {
  const auto& blades = bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).blades;
  bladePixels->entry()->SetRange(bladeSelect->GetSelection() >= 0 ? blades.at(bladeSelect->GetSelection()).subBlades.size() : 1, parent->generalPage->maxLEDs->entry()->GetValue());

  if (subBladeSelect ->GetSelection() >= 0 && blades.at(bladeSelect->GetSelection()).isSubBlade) {
    subBladeStart->entry()->SetRange(0, blades.at(bladeSelect->GetSelection()).numPixels - 1);
    subBladeEnd->entry()->SetRange(blades.at(bladeSelect->GetSelection()).subBlades.at(subBladeSelect->GetSelection()).startPixel, blades.at(bladeSelect->GetSelection()).numPixels - 1);
  }
}

void BladesPage::addBlade() {
  bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.push_back(BladeConfig());
  update();
}
void BladesPage::addSubBlade() {
  bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).isSubBlade = true;
  bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.push_back(BladeConfig::subBladeInfo());
  if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.size() <= 1) bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.push_back(BladeConfig::subBladeInfo());
  update();
}
void BladesPage::removeBlade() {
  saveCurrent();
  
  if (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 1) {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.erase(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.begin() + lastBladeSelection);
  }

  update();
}
void BladesPage::removeSubBlade() {
  saveCurrent();

  if (BD_SUBHASSELECTION) {
    bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.erase(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.begin() + lastSubBladeSelection);
    if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.size() <= 1) {
      bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).subBlades.clear();
      bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(lastBladeSelection).isSubBlade = false;
    }
    lastSubBladeSelection = -1;
  }

  update();
}
