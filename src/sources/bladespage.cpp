#include "bladespage.h"

#include "misc.h"
#include "defines.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

BladesPage* BladesPage::instance;
BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  BladesPage::instance = this;

  wxWrapSizer* bladeSetup = new wxWrapSizer( wxVERTICAL);
  bladeType = new wxComboBox( GetStaticBox(), Misc::ID_BladeType, BD_PIXELRGB, wxDefaultPosition, wxDefaultSize, Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_TRISTAR, BD_QUADSTAR, BD_SINGLELED}), wxCB_READONLY);
  usePowerPin1 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 1");
  usePowerPin2 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 2");
  usePowerPin3 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 3");
  usePowerPin4 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 4");
  usePowerPin5 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 5");
  usePowerPin6 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 6");
  bladeSetup->Add(bladeType, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin1, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin2, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin3, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin4, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin5, MENUITEMFLAGS);
  bladeSetup->Add(usePowerPin6, MENUITEMFLAGS);

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
  star1Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star1Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star1->Add(star1ColorLabel, MENUITEMFLAGS);
  star1->Add(star1Color, MENUITEMFLAGS);
  star1->Add(star1Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star2 = new wxBoxSizer( wxVERTICAL);
  star2ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 2 Color");
  star2Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star2Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star2->Add(star2ColorLabel, MENUITEMFLAGS);
  star2->Add(star2Color, MENUITEMFLAGS);
  star2->Add(star2Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star3 = new wxBoxSizer( wxVERTICAL);
  star3ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 3 Color");
  star3Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star3Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star3->Add(star3ColorLabel, MENUITEMFLAGS);
  star3->Add(star3Color, MENUITEMFLAGS);
  star3->Add(star3Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star4 = new wxBoxSizer( wxVERTICAL);
  star4ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 4 Color");
  star4Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  star4Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
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

  Add(createBladeManager(), BOXITEMFLAGS);
  Add(bladeSetup, wxSizerFlags(0));
  Add(bladeSettings, wxSizerFlags(1));
}

wxBoxSizer* BladesPage::createBladeManager() {
  wxBoxSizer* bladeManager = new wxBoxSizer( wxHORIZONTAL);

  wxBoxSizer* bladeSelection = new wxBoxSizer( wxVERTICAL);
  wxStaticText* bladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "Blades");
  bladeSelect = new wxListBox( GetStaticBox(), Misc::ID_BladeSelect);
  wxBoxSizer* bladeButtons = new wxBoxSizer( wxHORIZONTAL);
  addBladeButton = new wxButton( GetStaticBox(), Misc::ID_AddBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  removeBladeButton = new wxButton( GetStaticBox(), Misc::ID_RemoveBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  bladeButtons->Add(addBladeButton, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  bladeButtons->Add(removeBladeButton, wxSizerFlags(0).Border(wxTOP, 10));
  bladeSelection->Add(bladeText, wxSizerFlags(0));
  bladeSelection->Add(bladeSelect, wxSizerFlags(1).Expand());
  bladeSelection->Add(bladeButtons, wxSizerFlags(0));

  wxBoxSizer* subBladeSelection = new wxBoxSizer( wxVERTICAL);
  wxStaticText* subBladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlades");
  subBladeSelect = new wxListBox( GetStaticBox(), Misc::ID_SubBladeSelect);
  wxBoxSizer* subBladeButtons = new wxBoxSizer( wxHORIZONTAL);
  addSubBladeButton = new wxButton( GetStaticBox(), Misc::ID_AddSubBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  removeSubBladeButton = new wxButton( GetStaticBox(), Misc::ID_RemoveSubBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  subBladeButtons->Add(addSubBladeButton, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  subBladeButtons->Add(removeSubBladeButton, wxSizerFlags(0).Border(wxTOP, 10));
  subBladeSelection->Add(subBladeText, wxSizerFlags(0));
  subBladeSelection->Add(subBladeSelect, wxSizerFlags(1).Expand());
  subBladeSelection->Add(subBladeButtons, wxSizerFlags(0));

  bladeManager->Add(bladeSelection, wxSizerFlags(0).Expand());
  bladeManager->Add(subBladeSelection, wxSizerFlags(0).Expand());

  return bladeManager;
}

void BladesPage::update() {
  saveCurrent();
  rebuildBladeArray();
  loadSettings();
  setEnabled();
  setVisibility();
}

void BladesPage::saveCurrent() {
  if (lastBladeSelection < 0 || lastBladeSelection >= (int32_t)blades.size()) return;

  blades.at(lastBladeSelection).type = bladeType->GetValue();
  blades.at(lastBladeSelection).usePowerPin1 = usePowerPin1->GetValue();
  blades.at(lastBladeSelection).usePowerPin2 = usePowerPin2->GetValue();
  blades.at(lastBladeSelection).usePowerPin3 = usePowerPin3->GetValue();
  blades.at(lastBladeSelection).usePowerPin4 = usePowerPin4->GetValue();
  blades.at(lastBladeSelection).usePowerPin5 = usePowerPin5->GetValue();
  blades.at(lastBladeSelection).usePowerPin6 = usePowerPin6->GetValue();

  blades.at(lastBladeSelection).dataPin = bladeDataPin->GetValue();
  blades.at(lastBladeSelection).numPixels = bladePixels->GetValue();
  blades.at(lastBladeSelection).colorType = blades.at(lastBladeSelection).type == BD_PIXELRGB ? blade3ColorOrder->GetValue() : blade4ColorOrder->GetValue();
  blades.at(lastBladeSelection).useRGBWithWhite = blade4UseRGB->GetValue();

  blades.at(lastBladeSelection).Star1 = star1Color->GetValue();
  blades.at(lastBladeSelection).Star1Resistance = star1Resistance->num->GetValue();
  blades.at(lastBladeSelection).Star2 = star2Color->GetValue();
  blades.at(lastBladeSelection).Star2Resistance = star2Resistance->num->GetValue();
  blades.at(lastBladeSelection).Star3 = star3Color->GetValue();
  blades.at(lastBladeSelection).Star3Resistance = star3Resistance->num->GetValue();
  blades.at(lastBladeSelection).Star4 = star4Color->GetValue();
  blades.at(lastBladeSelection).Star4Resistance = star4Resistance->num->GetValue();

  if (lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(lastBladeSelection).subBlades.size()) {
    blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).startPixel = subBladeStart->GetValue();
    blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).endPixel = subBladeEnd->GetValue();
  }
  blades.at(lastBladeSelection).subBladeWithStride = subBladeUseStride->GetValue();

  // Check if SubBlades need to be removed (changed from WX281X)
  if (BD_HASSELECTION && lastBladeSelection == bladeSelect->GetSelection() && !BD_ISPIXEL) {
    blades.at(lastBladeSelection).isSubBlade = false;
    blades.at(lastBladeSelection).subBlades.clear();
  }
}
void BladesPage::rebuildBladeArray() {
  if (blades.size() == 0) blades.push_back(bladeConfig());

  lastBladeSelection = bladeSelect->GetSelection();
  bladeSelect->Clear();
  for (int32_t bladeNum = 0; bladeNum < (int32_t)blades.size(); bladeNum++)
    bladeSelect->Append("Blade " + std::to_string(bladeNum));
  if (lastBladeSelection >= 0 && lastBladeSelection < (int32_t)bladeSelect->GetCount()) bladeSelect->SetSelection(lastBladeSelection);

  lastSubBladeSelection = subBladeSelect->GetSelection();
  subBladeSelect->Clear();
  if (bladeSelect->GetSelection() >= 0 && bladeSelect->GetSelection() < (int32_t)blades.size()) {
    for (int32_t subBladeNum = 0; bladeSelect->GetSelection() != -1 && subBladeNum < (int32_t)blades.at(bladeSelect->GetSelection()).subBlades.size(); subBladeNum++)
      subBladeSelect->Append("SubBlade " + std::to_string(subBladeNum));
    if (lastSubBladeSelection >= 0 && lastSubBladeSelection < (int32_t)subBladeSelect->GetCount()) subBladeSelect->SetSelection(lastSubBladeSelection);
  }
}
void BladesPage::loadSettings() {
  if (bladeSelect->GetSelection() < 0 || bladeSelect->GetSelection() >= (int32_t)blades.size())
    return;

  bladeType->SetValue(blades.at(bladeSelect->GetSelection()).type);
  usePowerPin1->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin1);
  usePowerPin2->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin2);
  usePowerPin3->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin3);
  usePowerPin4->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin4);
  usePowerPin5->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin5);
  usePowerPin6->SetValue(blades.at(bladeSelect->GetSelection()).usePowerPin6);

  bladeDataPin->SetValue(blades.at(bladeSelect->GetSelection()).dataPin);
  bladePixels->SetValue(blades.at(bladeSelect->GetSelection()).numPixels);
  blade3ColorOrder->SetValue(blades.at(bladeSelect->GetSelection()).colorType);
  blade4ColorOrder->SetValue(blades.at(bladeSelect->GetSelection()).colorType);
  blade4UseRGB->SetValue(blades.at(bladeSelect->GetSelection()).useRGBWithWhite);

  star1Color->SetValue(blades.at(bladeSelect->GetSelection()).Star1);
  star1Resistance->num->SetValue(blades.at(bladeSelect->GetSelection()).Star1Resistance);
  star2Color->SetValue(blades.at(bladeSelect->GetSelection()).Star2);
  star2Resistance->num->SetValue(blades.at(bladeSelect->GetSelection()).Star2Resistance);
  star3Color->SetValue(blades.at(bladeSelect->GetSelection()).Star3);
  star3Resistance->num->SetValue(blades.at(bladeSelect->GetSelection()).Star3Resistance);
  star4Color->SetValue(blades.at(bladeSelect->GetSelection()).Star4);
  star4Resistance->num->SetValue(blades.at(bladeSelect->GetSelection()).Star4Resistance);

  subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(bladeSelect->GetSelection()).subBlades.size() ? blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).startPixel : 0);
  subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(bladeSelect->GetSelection()).subBlades.size() ? blades.at(bladeSelect->GetSelection()).subBlades.at(lastSubBladeSelection).endPixel : 0);
  subBladeUseStride->SetValue(blades.at(bladeSelect->GetSelection()).subBladeWithStride);
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
  star1Resistance->num->Enable(star1Color->GetStringSelection() != BD_NORESISTANCE);
  star2ColorLabel->Show(BD_ISSTAR);
  star2Color->Show(BD_ISSTAR);
  star2Resistance->box->Show(BD_ISSTAR);
  star2Resistance->num->Enable(star2Color->GetStringSelection() != BD_NORESISTANCE);
  star3ColorLabel->Show(BD_ISSTAR);
  star3Color->Show(BD_ISSTAR);
  star3Resistance->box->Show(BD_ISSTAR);
  star3Resistance->num->Enable(star3Color->GetStringSelection() != BD_NORESISTANCE);
  star4ColorLabel->Show(BD_ISSTAR4);
  star4Color->Show(BD_ISSTAR4);
  star4Resistance->box->Show(BD_ISSTAR4);
  star4Resistance->num->Enable(star4Color->GetStringSelection() != BD_NORESISTANCE);

  subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
  subBladeStartLabel->Show(BD_SUBHASSELECTION);
  subBladeStart->Show(BD_SUBHASSELECTION);
  subBladeEndLabel->Show(BD_SUBHASSELECTION);
  subBladeEnd->Show(BD_SUBHASSELECTION);
}

void BladesPage::addBlade() {
  blades.push_back(bladeConfig());
  update();
}
void BladesPage::addSubBlade() {
  blades.at(lastBladeSelection).isSubBlade = true;
  blades.at(lastBladeSelection).subBlades.push_back(bladeConfig::subBladeInfo());
  if (blades.at(lastBladeSelection).subBlades.size() <= 1) blades.at(lastBladeSelection).subBlades.push_back(bladeConfig::subBladeInfo());
  update();
}
void BladesPage::removeBlade() {
  saveCurrent();

  if (BD_HASSELECTION && blades.size() > 1) {
    blades.erase(blades.begin() + lastBladeSelection);
  }

  update();
}
void BladesPage::removeSubBlade() {
  saveCurrent();

  if (BD_SUBHASSELECTION) {
    blades.at(lastBladeSelection).subBlades.erase(blades.at(lastBladeSelection).subBlades.begin() + lastSubBladeSelection);
    if (blades.at(lastBladeSelection).subBlades.size() <= 1) {
      blades.at(lastBladeSelection).subBlades.clear();
      blades.at(lastBladeSelection).isSubBlade = false;
    }
    lastSubBladeSelection = -1;
  }

  update();
}
