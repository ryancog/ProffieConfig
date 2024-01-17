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
  Add(createBladeSetup(), wxSizerFlags(0));
  Add(createBladeSettings(), wxSizerFlags(1));

  bindEvents();
  createToolTips();
}

void BladesPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { if (bladeArrayDlg->IsShown()) bladeArrayDlg->Raise(); else bladeArrayDlg->Show(); }, ID_OpenBladeArrays);

  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { parent->presetsPage->bladeArray->entry()->SetSelection(bladeArray->entry()->GetSelection()); update(); }, ID_BladeArray);
  GetStaticBox()->Bind(wxEVT_SPINCTRL, [&](wxCommandEvent& event) { update(); event.Skip(); });
  GetStaticBox()->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent& event) { update(); event.Skip(); });
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW(parent);
      }, ID_BladeSelect);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW(parent);
      }, ID_SubBladeSelect);
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW(parent);
      }, ID_BladeType);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addBlade(); UPDATEWINDOW(parent); }, ID_AddBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addSubBlade(); FULLUPDATEWINDOW(parent); }, ID_AddSubBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeBlade(); UPDATEWINDOW(parent); }, ID_RemoveBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeSubBlade(); UPDATEWINDOW(parent); }, ID_RemoveSubBlade);
}
void BladesPage::createToolTips() {
  TIP(bladeArray, "The currently-selected Blade Array to edit.");
  TIP(addBladeButton, "Add a blade to the selected blade array.");
  TIP(removeBladeButton, "Remove the currently-selected blade.");
  TIP(addSubBladeButton, "Add a Sub-Blade to the currently-selected blade.\nCan only be used with WS281X-type blades.");
  TIP(removeSubBladeButton, "Remove the currently-selected Sub-Blade.\nIf there are less than 2 Sub-Blades after removal, the remaining Sub-Blade will be deleted.");

  TIP(bladeType, "The type of blade/LED.");
  TIP(usePowerPin1, "Use Power Pin 1 to power this blade");
  TIP(usePowerPin2, "Use Power Pin 2 to power this blade");
  TIP(usePowerPin3, "Use Power Pin 3 to power this blade");
  TIP(usePowerPin4, "Use Power Pin 4 to power this blade");
  TIP(usePowerPin5, "Use Power Pin 5 to power this blade");
  TIP(usePowerPin6, "Use Power Pin 6 to power this blade");

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
  bladeButtons->Add(addBladeButton, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  bladeButtons->Add(removeBladeButton, wxSizerFlags(0).Border(wxTOP, 10));
  bladeSelectionSizer->Add(bladeText, wxSizerFlags(0));
  bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand());
  bladeSelectionSizer->Add(bladeButtons, wxSizerFlags(0).Center());

  wxBoxSizer* subBladeSelectionSizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* subBladeText = new wxStaticText(GetStaticBox(), wxID_ANY, "SubBlades");
  subBladeSelect = new wxListBox(GetStaticBox(), ID_SubBladeSelect, wxDefaultPosition, wxSize(100, -1));
  wxBoxSizer* subBladeButtonSizer = new wxBoxSizer(wxHORIZONTAL);
  addSubBladeButton = new wxButton(GetStaticBox(), ID_AddSubBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeSubBladeButton = new wxButton(GetStaticBox(), ID_RemoveSubBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  subBladeButtonSizer->Add(addSubBladeButton, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  subBladeButtonSizer->Add(removeSubBladeButton, wxSizerFlags(0).Border(wxTOP, 10));
  subBladeSelectionSizer->Add(subBladeText, wxSizerFlags(0));
  subBladeSelectionSizer->Add(subBladeSelect, wxSizerFlags(1).Expand());
  subBladeSelectionSizer->Add(subBladeButtonSizer, wxSizerFlags(0).Center());

  bladeManagerSizer->Add(bladeSelectionSizer, wxSizerFlags(1).Expand());
  bladeManagerSizer->Add(subBladeSelectionSizer, wxSizerFlags(1).Expand());

  return bladeManagerSizer;
}
wxBoxSizer* BladesPage::createBladeSetup() {
  wxBoxSizer* bladeSetup = new wxBoxSizer(wxVERTICAL);
  bladeType = new pcComboBox(GetStaticBox(), ID_BladeType, "Blade Type", wxDefaultPosition, wxDefaultSize, Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_TRISTAR, BD_QUADSTAR, BD_SINGLELED}), wxCB_READONLY);
  usePowerPin1 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 1");
  usePowerPin2 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 2");
  usePowerPin3 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 3");
  usePowerPin4 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 4");
  usePowerPin5 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 5");
  usePowerPin6 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 6");
  bladeSetup->Add(bladeType, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin1, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin2, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin3, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin4, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin5, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin6, MENUITEMFLAGS);

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
  star1->Add(star1Resistance, MENUITEMFLAGS);

  wxBoxSizer* star2 = new wxBoxSizer(wxVERTICAL);
  star2Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 2 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star2Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star2->Add(star2Color, MENUITEMFLAGS);
  star2->Add(star2Resistance, MENUITEMFLAGS);

  wxBoxSizer* star3 = new wxBoxSizer(wxVERTICAL);
  star3Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 3 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star3Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star3->Add(star3Color, MENUITEMFLAGS);
  star3->Add(star3Resistance, MENUITEMFLAGS);

  wxBoxSizer* star4 = new wxBoxSizer(wxVERTICAL);
  star4Color = new pcComboBox(GetStaticBox(), wxID_ANY, "LED 4 Color", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star4Resistance = new pcSpinCtrl(GetStaticBox(), wxID_ANY, "Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, 500);
  star4->Add(star4Color, MENUITEMFLAGS);
  star4->Add(star4Resistance, MENUITEMFLAGS);

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
  
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).type = bladeType->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin1 = usePowerPin1->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin2 = usePowerPin2->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin3 = usePowerPin3->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin4 = usePowerPin4->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin5 = usePowerPin5->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin6 = usePowerPin6->GetValue();
  
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).dataPin = bladeDataPin->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).numPixels = bladePixels->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).colorType = bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).type == BD_PIXELRGB ? blade3ColorOrder->entry()->GetValue() : blade4ColorOrder->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).useRGBWithWhite = blade4UseRGB->GetValue();
  
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star1 = star1Color->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star1Resistance = star1Resistance->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star2 = star2Color->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star2Resistance = star2Resistance->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star3 = star3Color->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star3Resistance = star3Resistance->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star4 = star4Color->entry()->GetValue();
  bladeArrayDlg->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star4Resistance = star4Resistance->entry()->GetValue();
  
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
  if (bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() == 0) bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.push_back(BladeConfig({ .numPixels = 144, .usePowerPin2 = true, .usePowerPin3 = true }));

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
  if (bladeSelect->GetSelection() < 0 || bladeSelect->GetSelection() >= (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size())
    return;
  
  bladeType->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).type);
  usePowerPin1->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin1);
  usePowerPin2->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin2);
  usePowerPin3->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin3);
  usePowerPin4->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin4);
  usePowerPin5->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin5);
  usePowerPin6->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin6);
  
  bladeDataPin->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).dataPin);
  bladePixels->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).numPixels);
  blade3ColorOrder->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).colorType);
  blade4ColorOrder->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).colorType);
  blade4UseRGB->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).useRGBWithWhite);
  
  star1Color->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star1);
  star1Resistance->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star1Resistance);
  star2Color->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star2);
  star2Resistance->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star2Resistance);
  star3Color->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star3);
  star3Resistance->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star3Resistance);
  star4Color->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star4);
  star4Resistance->entry()->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).Star4Resistance);
  
  subBladeStart->entry()->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size() ? bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).startPixel : 0);
  subBladeEnd->entry()->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size() ? bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).endPixel : 0);
  useStride->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).useStride);
  useZigZag->SetValue(bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(bladeSelect->GetSelection()).useZigZag);
}
void BladesPage::setEnabled() {
  removeBladeButton->Enable(bladeSelect->GetCount() > 1 && BD_HASSELECTION);
  removeSubBladeButton->Enable(subBladeSelect->GetCount() > 0 && BD_SUBHASSELECTION);
  addSubBladeButton->Enable(BD_ISPIXEL && BD_HASSELECTION);

  bladeType->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin1->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin2->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin3->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin4->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin5->Enable(BD_HASSELECTION && BD_ISFIRST);
  usePowerPin6->Enable(BD_HASSELECTION && BD_ISFIRST);
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
  star1Resistance->entry()->Enable(star1Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star2Color->Show(BD_ISSTAR);
  star2Resistance->Show(BD_ISSTAR);
  star2Resistance->entry()->Enable(star2Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star3Color->Show(BD_ISSTAR);
  star3Resistance->Show(BD_ISSTAR);
  star3Resistance->entry()->Enable(star3Color->entry()->GetStringSelection() != BD_NORESISTANCE);
  star4Color->Show(BD_ISSTAR4);
  star4Resistance->Show(BD_ISSTAR4);
  star4Resistance->entry()->Enable(star4Color->entry()->GetStringSelection() != BD_NORESISTANCE);

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
