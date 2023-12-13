// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "bladespage.h"

#include "mainwindow.h"
#include "misc.h"
#include "defines.h"
#include "bladeidpage.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/tooltip.h>

BladesPage* BladesPage::instance;
BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  BladesPage::instance = this;

  Add(createBladeSelect(), wxSizerFlags(0).Expand());
  Add(createBladeSetup(), wxSizerFlags(0));
  Add(createBladeSettings(), wxSizerFlags(1));

  bindEvents();
  createToolTips();
}

void BladesPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { PresetsPage::instance->bladeArray->SetSelection(bladeArray->GetSelection()); update(); }, ID_BladeArray);

  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW;
      }, ID_BladeSelect);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW;
      }, ID_SubBladeSelect);
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW;
      }, ID_BladeType);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addBlade(); UPDATEWINDOW; }, ID_AddBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { addSubBlade(); FULLUPDATEWINDOW; }, ID_AddSubBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeBlade(); UPDATEWINDOW; }, ID_RemoveBlade);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { removeSubBlade(); UPDATEWINDOW; }, ID_RemoveSubBlade);
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

  TIP(subBladeUseStride, "See the POD Page \"SubBlade\" for more info.\nMost commonly used with \"KR Pixel Stick\" blades.");
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
  bladeArray = new wxComboBox(GetStaticBox(), ID_BladeArray, "blade_in", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ "blade_in" }), wxCB_READONLY);
  wxStaticText* bladeArrayText = new wxStaticText(GetStaticBox(), wxID_ANY, "Blade Array", wxDefaultPosition, wxDefaultSize, 0);
  bladeSelectSizer->Add(bladeArrayText, TEXTITEMFLAGS);
  bladeSelectSizer->Add(bladeArray, TEXTITEMFLAGS.Expand());
  bladeSelectSizer->Add(createBladeManager(), wxSizerFlags(1).Border(wxALL, 5).Expand());

  return bladeSelectSizer;
}
wxBoxSizer* BladesPage::createBladeManager() {
  wxBoxSizer* bladeManagerSizer = new wxBoxSizer( wxHORIZONTAL);

  wxBoxSizer* bladeSelectionSizer = new wxBoxSizer( wxVERTICAL);
  wxStaticText* bladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "Blades");
  bladeSelect = new wxListBox( GetStaticBox(), ID_BladeSelect);
  wxBoxSizer* bladeButtons = new wxBoxSizer( wxHORIZONTAL);
  addBladeButton = new wxButton( GetStaticBox(), ID_AddBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeBladeButton = new wxButton( GetStaticBox(), ID_RemoveBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  bladeButtons->Add(addBladeButton, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  bladeButtons->Add(removeBladeButton, wxSizerFlags(0).Border(wxTOP, 10));
  bladeSelectionSizer->Add(bladeText, wxSizerFlags(0));
  bladeSelectionSizer->Add(bladeSelect, wxSizerFlags(1).Expand());
  bladeSelectionSizer->Add(bladeButtons, wxSizerFlags(0).Center());

  wxBoxSizer* subBladeSelectionSizer = new wxBoxSizer( wxVERTICAL);
  wxStaticText* subBladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlades");
  subBladeSelect = new wxListBox( GetStaticBox(), ID_SubBladeSelect);
  wxBoxSizer* subBladeButtonSizer = new wxBoxSizer( wxHORIZONTAL);
  addSubBladeButton = new wxButton( GetStaticBox(), ID_AddSubBlade, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeSubBladeButton = new wxButton( GetStaticBox(), ID_RemoveSubBlade, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
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
  bladeType = new wxComboBox( GetStaticBox(), ID_BladeType, BD_PIXELRGB, wxDefaultPosition, wxDefaultSize, Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_TRISTAR, BD_QUADSTAR, BD_SINGLELED}), wxCB_READONLY);
  usePowerPin1 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 1");
  usePowerPin2 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 2");
  usePowerPin3 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 3");
  usePowerPin4 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 4");
  usePowerPin5 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 5");
  usePowerPin6 = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Power Pin 6");
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
  wxWrapSizer* bladeSettings = new wxWrapSizer( wxVERTICAL);
  wxBoxSizer* bladeColor = new wxBoxSizer( wxVERTICAL);
  bladeColorOrderLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Color Order");
  blade3ColorOrder = new wxComboBox( GetStaticBox(), wxID_ANY, "GRB", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}), wxCB_READONLY);
  blade4ColorOrder = new wxComboBox( GetStaticBox(), wxID_ANY, "GRBW", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}), wxCB_READONLY);
  bladeColor->Add(bladeColorOrderLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeColor->Add(blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeColor->Add(blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  blade4UseRGB = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use RGB with White");
  bladeDataPinLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Blade Data Pin");
  bladeDataPin = new wxComboBox( GetStaticBox(), wxID_ANY, "bladePin", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}));
  bladePixelsLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Number of Pixels");
  bladePixels = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

  wxBoxSizer* star1 = new wxBoxSizer( wxVERTICAL);
  star1ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 1 Color");
  star1Color = new wxComboBox( GetStaticBox(), wxID_ANY, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star1Resistance = Misc::createNumEntry(this->GetStaticBox(), "Resistance", wxID_ANY, 0, 10000, 500);
  star1->Add(star1ColorLabel, MENUITEMFLAGS);
  star1->Add(star1Color, MENUITEMFLAGS);
  star1->Add(star1Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star2 = new wxBoxSizer( wxVERTICAL);
  star2ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 2 Color");
  star2Color = new wxComboBox( GetStaticBox(), wxID_ANY, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star2Resistance = Misc::createNumEntry(this->GetStaticBox(), "Resistance", wxID_ANY, 0, 10000, 500);
  star2->Add(star2ColorLabel, MENUITEMFLAGS);
  star2->Add(star2Color, MENUITEMFLAGS);
  star2->Add(star2Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star3 = new wxBoxSizer( wxVERTICAL);
  star3ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 3 Color");
  star3Color = new wxComboBox( GetStaticBox(), wxID_ANY, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star3Resistance = Misc::createNumEntry(this->GetStaticBox(), "Resistance", wxID_ANY, 0, 10000, 500);
  star3->Add(star3ColorLabel, MENUITEMFLAGS);
  star3->Add(star3Color, MENUITEMFLAGS);
  star3->Add(star3Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star4 = new wxBoxSizer( wxVERTICAL);
  star4ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 4 Color");
  star4Color = new wxComboBox( GetStaticBox(), wxID_ANY, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star4Resistance = Misc::createNumEntry(this->GetStaticBox(), "Resistance", wxID_ANY, 0, 10000, 500);
  star4->Add(star4ColorLabel, MENUITEMFLAGS);
  star4->Add(star4Color, MENUITEMFLAGS);
  star4->Add(star4Resistance->box, MENUITEMFLAGS);

  subBladeUseStride = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Stride for SubBlade");
  subBladeStartLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlade Start");
  subBladeStart = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
  subBladeEndLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlade End");
  subBladeEnd = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
  //bladeID

  bladeSettings->Add(bladeColor);
  bladeSettings->Add(blade4UseRGB, MENUITEMFLAGS);
  bladeSettings->Add(star1, MENUITEMFLAGS);
  bladeSettings->Add(star2, MENUITEMFLAGS);
  bladeSettings->Add(star3, MENUITEMFLAGS);
  bladeSettings->Add(star4, MENUITEMFLAGS);
  bladeSettings->Add(bladeDataPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(bladePixelsLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(bladePixels, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  bladeSettings->Add(subBladeUseStride, MENUITEMFLAGS);
  bladeSettings->Add(subBladeStartLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(subBladeStart, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(subBladeEndLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(subBladeEnd, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  return bladeSettings;
}

void BladesPage::update() {
  saveCurrent();
  rebuildBladeArray();
  loadSettings();
  setEnabled();
  setVisibility();
}

void BladesPage::saveCurrent() {
  if (lastBladeSelection < 0 || lastBladeSelection >= (int32_t)BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size()) return;

  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).type = bladeType->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin1 = usePowerPin1->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin2 = usePowerPin2->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin3 = usePowerPin3->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin4 = usePowerPin4->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin5 = usePowerPin5->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).usePowerPin6 = usePowerPin6->GetValue();

  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).dataPin = bladeDataPin->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).numPixels = bladePixels->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).colorType = BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).type == BD_PIXELRGB ? blade3ColorOrder->GetValue() : blade4ColorOrder->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).useRGBWithWhite = blade4UseRGB->GetValue();

  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star1 = star1Color->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star1Resistance = star1Resistance->num->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star2 = star2Color->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star2Resistance = star2Resistance->num->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star3 = star3Color->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star3Resistance = star3Resistance->num->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star4 = star4Color->GetValue();
  BladeIDPage::instance->bladeArrays[lastBladeArraySelection].blades.at(lastBladeSelection).Star4Resistance = star4Resistance->num->GetValue();

  if (lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.size()) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).startPixel = subBladeStart->GetValue();
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).endPixel = subBladeEnd->GetValue();
  }
  BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBladeWithStride = subBladeUseStride->GetValue();

  // Check if SubBlades need to be removed (changed from WX281X)
  if (BD_HASSELECTION && lastBladeSelection == bladeSelect->GetSelection() && !BD_ISPIXEL) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).isSubBlade = false;
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.clear();
  }
}
void BladesPage::rebuildBladeArray() {
  if (BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() == 0) BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.push_back(BladeConfig());

  lastBladeSelection = bladeSelect->GetSelection();
  lastSubBladeSelection = subBladeSelect->GetSelection();
  lastBladeArraySelection = bladeArray->GetSelection();

  bladeArray->Clear();
  for (BladeIDPage::BladeArray& array : BladeIDPage::instance->bladeArrays) {
    bladeArray->Append(array.name);
  }
  if (lastBladeArraySelection >= 0 && lastBladeArraySelection < static_cast<int32_t>(bladeArray->GetCount())) bladeArray->SetSelection(lastBladeArraySelection);
  else bladeArray->SetSelection(0);

  bladeSelect->Clear();
  for (int32_t bladeNum = 0; bladeNum < static_cast<int32_t>(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size()); bladeNum++)
    bladeSelect->Append("Blade " + std::to_string(bladeNum));
  if (lastBladeSelection >= 0 && lastBladeSelection < static_cast<int32_t>(bladeSelect->GetCount())) bladeSelect->SetSelection(lastBladeSelection);

  subBladeSelect->Clear();
  if (bladeSelect->GetSelection() >= 0 && bladeSelect->GetSelection() < static_cast<int32_t>(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size())) {
    for (int32_t subBladeNum = 0; bladeSelect->GetSelection() != -1 && subBladeNum < static_cast<int32_t>(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size()); subBladeNum++)
      subBladeSelect->Append("SubBlade " + std::to_string(subBladeNum));
    if (lastSubBladeSelection >= 0 && lastSubBladeSelection < static_cast<int32_t>(subBladeSelect->GetCount())) subBladeSelect->SetSelection(lastSubBladeSelection);
  }
}
void BladesPage::loadSettings() {
  if (bladeSelect->GetSelection() < 0 || bladeSelect->GetSelection() >= (int32_t)BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size())
    return;

  bladeType->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).type);
  usePowerPin1->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin1);
  usePowerPin2->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin2);
  usePowerPin3->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin3);
  usePowerPin4->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin4);
  usePowerPin5->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin5);
  usePowerPin6->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).usePowerPin6);

  bladeDataPin->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).dataPin);
  bladePixels->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).numPixels);
  blade3ColorOrder->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).colorType);
  blade4ColorOrder->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).colorType);
  blade4UseRGB->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).useRGBWithWhite);

  star1Color->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star1);
  star1Resistance->num->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star1Resistance);
  star2Color->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star2);
  star2Resistance->num->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star2Resistance);
  star3Color->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star3);
  star3Resistance->num->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star3Resistance);
  star4Color->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star4);
  star4Resistance->num->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).Star4Resistance);

  subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size() ? BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).startPixel : 0);
  subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.size() ? BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).endPixel : 0);
  subBladeUseStride->SetValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(bladeSelect->GetSelection()).subBladeWithStride);
}
void BladesPage::setEnabled() {
  removeBladeButton->Enable(bladeSelect->GetCount() > 0 && BD_HASSELECTION);
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
  bladeColorOrderLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  blade3ColorOrder->Show(BD_ISPIXEL3 && BD_ISFIRST);
  blade4ColorOrder->Show(BD_ISPIXEL4 && BD_ISFIRST);
  blade4UseRGB->Show(BD_ISPIXEL4 && BD_ISFIRST);

  bladeDataPinLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  bladeDataPin->Show(BD_ISPIXEL && BD_ISFIRST);
  bladePixelsLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  bladePixels->Show(BD_ISPIXEL && BD_ISFIRST);

  star1ColorLabel->Show(BD_ISSTAR);
  star1Color->Show(BD_ISSTAR);
  star1Resistance->box->Show(BD_ISSTAR);
  star1Resistance->Enable(star1Color->GetStringSelection() != BD_NORESISTANCE);
  star2ColorLabel->Show(BD_ISSTAR);
  star2Color->Show(BD_ISSTAR);
  star2Resistance->box->Show(BD_ISSTAR);
  star2Resistance->Enable(star2Color->GetStringSelection() != BD_NORESISTANCE);
  star3ColorLabel->Show(BD_ISSTAR);
  star3Color->Show(BD_ISSTAR);
  star3Resistance->box->Show(BD_ISSTAR);
  star3Resistance->Enable(star3Color->GetStringSelection() != BD_NORESISTANCE);
  star4ColorLabel->Show(BD_ISSTAR4);
  star4Color->Show(BD_ISSTAR4);
  star4Resistance->box->Show(BD_ISSTAR4);
  star4Resistance->Enable(star4Color->GetStringSelection() != BD_NORESISTANCE);

  subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
  subBladeStartLabel->Show(BD_SUBHASSELECTION);
  subBladeStart->Show(BD_SUBHASSELECTION);
  subBladeEndLabel->Show(BD_SUBHASSELECTION);
  subBladeEnd->Show(BD_SUBHASSELECTION);
}

void BladesPage::addBlade() {
  BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.push_back(BladeConfig());
  update();
}
void BladesPage::addSubBlade() {
  BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).isSubBlade = true;
  BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.push_back(BladeConfig::subBladeInfo());
  if (BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.size() <= 1) BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.push_back(BladeConfig::subBladeInfo());
  update();
}
void BladesPage::removeBlade() {
  saveCurrent();

  if (BD_HASSELECTION && BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() > 1) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.erase(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.begin() + lastBladeSelection);
  }

  update();
}
void BladesPage::removeSubBlade() {
  saveCurrent();

  if (BD_SUBHASSELECTION) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.erase(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.begin() + lastSubBladeSelection);
    if (BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.size() <= 1) {
      BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).subBlades.clear();
      BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(lastBladeSelection).isSubBlade = false;
    }
    lastSubBladeSelection = -1;
  }

  update();
}
