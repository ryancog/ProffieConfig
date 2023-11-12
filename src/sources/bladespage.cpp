#include "bladespage.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "misc.h"
#include "defines.h"

BladesPage* BladesPage::instance;
BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  BladesPage::instance = this;
  wxBoxSizer* bladeManager = new wxBoxSizer( wxHORIZONTAL);

  wxBoxSizer* bladeSelection = new wxBoxSizer( wxVERTICAL);
  wxStaticText* bladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "Blades");
  settings.bladeSelect = new wxListBox( GetStaticBox(), Misc::ID_BladeSelect);
  wxBoxSizer* bladeButtons = new wxBoxSizer( wxHORIZONTAL);
  settings.addBlade = new wxButton( GetStaticBox(), Misc::ID_AddBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  settings.removeBlade = new wxButton( GetStaticBox(), Misc::ID_RemoveBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  bladeButtons->Add(settings.addBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  bladeButtons->Add(settings.removeBlade, wxSizerFlags(0).Border(wxTOP, 10));
  bladeSelection->Add(bladeText, wxSizerFlags(0));
  bladeSelection->Add(settings.bladeSelect, wxSizerFlags(1).Expand());
  bladeSelection->Add(bladeButtons, wxSizerFlags(0));

  wxBoxSizer* subBladeSelection = new wxBoxSizer( wxVERTICAL);
  wxStaticText* subBladeText = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlades");
  settings.subBladeSelect = new wxListBox( GetStaticBox(), Misc::ID_SubBladeSelect);
  wxBoxSizer* subBladeButtons = new wxBoxSizer( wxHORIZONTAL);
  settings.addSubBlade = new wxButton( GetStaticBox(), Misc::ID_AddSubBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  settings.removeSubBlade = new wxButton( GetStaticBox(), Misc::ID_RemoveSubBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  subBladeButtons->Add(settings.addSubBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
  subBladeButtons->Add(settings.removeSubBlade, wxSizerFlags(0).Border(wxTOP, 10));
  subBladeSelection->Add(subBladeText, wxSizerFlags(0));
  subBladeSelection->Add(settings.subBladeSelect, wxSizerFlags(1).Expand());
  subBladeSelection->Add(subBladeButtons, wxSizerFlags(0));

  bladeManager->Add(bladeSelection, wxSizerFlags(0).Expand());
  bladeManager->Add(subBladeSelection, wxSizerFlags(0).Expand());


  wxWrapSizer* bladeSetup = new wxWrapSizer( wxVERTICAL);
  settings.bladeType = new wxComboBox( GetStaticBox(), Misc::ID_BladeType, BD_PIXELRGB, wxDefaultPosition, wxDefaultSize, Misc::createEntries({BD_PIXELRGB, BD_PIXELRGBW, BD_TRISTAR, BD_QUADSTAR, BD_SINGLELED}), wxCB_READONLY);
  settings.usePowerPin1 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 1");
  settings.usePowerPin2 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 2");
  settings.usePowerPin3 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 3");
  settings.usePowerPin4 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 4");
  settings.usePowerPin5 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 5");
  settings.usePowerPin6 = new wxCheckBox( GetStaticBox(), Misc::ID_BladePower, "Use Power Pin 6");
  bladeSetup->Add(settings.bladeType, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin1, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin2, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin3, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin4, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin5, MENUITEMFLAGS);
  bladeSetup->Add(settings.usePowerPin6, MENUITEMFLAGS);

  wxWrapSizer* bladeSettings = new wxWrapSizer( wxVERTICAL);

  wxBoxSizer* bladeColor = new wxBoxSizer( wxVERTICAL);
  settings.bladeColorOrderLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Color Order");
  settings.blade3ColorOrder = new wxComboBox( GetStaticBox(), wxID_ANY, "GRB", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}), wxCB_READONLY);
  settings.blade4ColorOrder = new wxComboBox( GetStaticBox(), wxID_ANY, "GRBW", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}), wxCB_READONLY);
  bladeColor->Add(settings.bladeColorOrderLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeColor->Add(settings.blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeColor->Add(settings.blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  settings.blade4UseRGB = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use RGB with White");
  settings.bladeDataPinLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Blade Data Pin");
  settings.bladeDataPin = new wxComboBox( GetStaticBox(), wxID_ANY, "bladePin", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}));
  settings.bladePixelsLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "Number of Pixels");
  settings.bladePixels = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

  wxBoxSizer* star1 = new wxBoxSizer( wxVERTICAL);
  settings.star1ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 1 Color");
  settings.star1Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  settings.star1Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star1->Add(settings.star1ColorLabel, MENUITEMFLAGS);
  star1->Add(settings.star1Color, MENUITEMFLAGS);
  star1->Add(settings.star1Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star2 = new wxBoxSizer( wxVERTICAL);
  settings.star2ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 2 Color");
  settings.star2Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  settings.star2Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star2->Add(settings.star2ColorLabel, MENUITEMFLAGS);
  star2->Add(settings.star2Color, MENUITEMFLAGS);
  star2->Add(settings.star2Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star3 = new wxBoxSizer( wxVERTICAL);
  settings.star3ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 3 Color");
  settings.star3Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  settings.star3Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star3->Add(settings.star3ColorLabel, MENUITEMFLAGS);
  star3->Add(settings.star3Color, MENUITEMFLAGS);
  star3->Add(settings.star3Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star4 = new wxBoxSizer( wxVERTICAL);
  settings.star4ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 4 Color");
  settings.star4Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, BD_NORESISTANCE, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", BD_NORESISTANCE}), wxCB_READONLY);
  settings.star4Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star4->Add(settings.star4ColorLabel, MENUITEMFLAGS);
  star4->Add(settings.star4Color, MENUITEMFLAGS);
  star4->Add(settings.star4Resistance->box, MENUITEMFLAGS);

  settings.subBladeUseStride = new wxCheckBox( GetStaticBox(), wxID_ANY, "Use Stride for SubBlade");
  settings.subBladeStartLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlade Start");
  settings.subBladeStart = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
  settings.subBladeEndLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "SubBlade End");
  settings.subBladeEnd = new wxSpinCtrl( GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
  //settings.bladeID

  bladeSettings->Add(bladeColor);
  bladeSettings->Add(settings.blade4UseRGB, MENUITEMFLAGS);
  bladeSettings->Add(star1, MENUITEMFLAGS);
  bladeSettings->Add(star2, MENUITEMFLAGS);
  bladeSettings->Add(star3, MENUITEMFLAGS);
  bladeSettings->Add(star4, MENUITEMFLAGS);
  bladeSettings->Add(settings.bladeDataPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.bladePixelsLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.bladePixels, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  bladeSettings->Add(settings.subBladeUseStride, MENUITEMFLAGS);
  bladeSettings->Add(settings.subBladeStartLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.subBladeStart, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.subBladeEndLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeSettings->Add(settings.subBladeEnd, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  Add(bladeManager, BOXITEMFLAGS);
  Add(bladeSetup, wxSizerFlags(0));
  Add(bladeSettings, wxSizerFlags(1));
}

void BladesPage::update() {
  lastBladeSelection = settings.bladeSelect->GetSelection();
  lastSubBladeSelection = settings.subBladeSelect->GetSelection();

  saveCurrent();
  rebuildBladeArray();
  loadSettings();
  setEnabled();
  setVisibility();
}

void BladesPage::saveCurrent() {
  if (lastBladeSelection >= 0 && lastBladeSelection < (int32_t)blades.size()) { // Save Options
    blades.at(lastBladeSelection).type = settings.bladeType->GetValue();
    blades.at(lastBladeSelection).usePowerPin1 = settings.usePowerPin1->GetValue();
    blades.at(lastBladeSelection).usePowerPin2 = settings.usePowerPin2->GetValue();
    blades.at(lastBladeSelection).usePowerPin3 = settings.usePowerPin3->GetValue();
    blades.at(lastBladeSelection).usePowerPin4 = settings.usePowerPin4->GetValue();
    blades.at(lastBladeSelection).usePowerPin5 = settings.usePowerPin5->GetValue();
    blades.at(lastBladeSelection).usePowerPin6 = settings.usePowerPin6->GetValue();

    blades.at(lastBladeSelection).dataPin = settings.bladeDataPin->GetValue();
    blades.at(lastBladeSelection).numPixels = settings.bladePixels->GetValue();
    blades.at(lastBladeSelection).colorType = blades.at(lastBladeSelection).type == BD_PIXELRGB ? settings.blade3ColorOrder->GetValue() : settings.blade4ColorOrder->GetValue();
    blades.at(lastBladeSelection).useRGBWithWhite = settings.blade4UseRGB->GetValue();

    blades.at(lastBladeSelection).Star1 = settings.star1Color->GetValue();
    blades.at(lastBladeSelection).Star1Resistance = settings.star1Resistance->num->GetValue();
    blades.at(lastBladeSelection).Star2 = settings.star2Color->GetValue();
    blades.at(lastBladeSelection).Star2Resistance = settings.star2Resistance->num->GetValue();
    blades.at(lastBladeSelection).Star3 = settings.star3Color->GetValue();
    blades.at(lastBladeSelection).Star3Resistance = settings.star3Resistance->num->GetValue();
    blades.at(lastBladeSelection).Star4 = settings.star4Color->GetValue();
    blades.at(lastBladeSelection).Star4Resistance = settings.star4Resistance->num->GetValue();

    if (lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(lastBladeSelection).subBlades.size()) {
      blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).startPixel = settings.subBladeStart->GetValue();
      blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).endPixel = settings.subBladeEnd->GetValue();
    }
    blades.at(lastBladeSelection).subBladeWithStride = settings.subBladeUseStride->GetValue();
  }

  // Check if SubBlades need to be removed (changed from WX281X)
  if (BD_HASSELECTION && lastBladeSelection == settings.bladeSelect->GetSelection() && !BD_ISPIXEL) {
    blades.at(lastBladeSelection).isSubBlade = false;
    blades.at(lastBladeSelection).subBlades.clear();
  }
}
void BladesPage::rebuildBladeArray() {
  if (blades.size() == 0) blades.push_back(bladeConfig());

  settings.bladeSelect->Clear();
  for (int32_t bladeNum = 0; bladeNum < (int32_t)blades.size(); bladeNum++)
    settings.bladeSelect->Append("Blade " + std::to_string(bladeNum));
  if ((int32_t)settings.bladeSelect->GetCount() - 1 >= lastBladeSelection) settings.bladeSelect->SetSelection(lastBladeSelection);

  settings.subBladeSelect->Clear();
  for (int32_t subBladeNum = 0; lastBladeSelection != -1 && subBladeNum < (int32_t)blades.at(lastBladeSelection).subBlades.size(); subBladeNum++)
    settings.subBladeSelect->Append("SubBlade " + std::to_string(subBladeNum));
  if ((int32_t)settings.subBladeSelect->GetCount() - 1 >= lastSubBladeSelection) settings.subBladeSelect->SetSelection(lastSubBladeSelection);
}
void BladesPage::loadSettings() {
  if (lastBladeSelection < 0 || lastBladeSelection >= (int32_t)blades.size())
    return;

  settings.bladeType->SetValue(blades.at(lastBladeSelection).type);
  settings.usePowerPin1->SetValue(blades.at(lastBladeSelection).usePowerPin1);
  settings.usePowerPin2->SetValue(blades.at(lastBladeSelection).usePowerPin2);
  settings.usePowerPin3->SetValue(blades.at(lastBladeSelection).usePowerPin3);
  settings.usePowerPin4->SetValue(blades.at(lastBladeSelection).usePowerPin4);
  settings.usePowerPin5->SetValue(blades.at(lastBladeSelection).usePowerPin5);
  settings.usePowerPin6->SetValue(blades.at(lastBladeSelection).usePowerPin6);

  settings.bladeDataPin->SetValue(blades.at(lastBladeSelection).dataPin);
  settings.bladePixels->SetValue(blades.at(lastBladeSelection).numPixels);
  settings.blade3ColorOrder->SetValue(blades.at(lastBladeSelection).colorType);
  settings.blade4ColorOrder->SetValue(blades.at(lastBladeSelection).colorType);
  settings.blade4UseRGB->SetValue(blades.at(lastBladeSelection).useRGBWithWhite);

  settings.star1Color->SetValue(blades.at(lastBladeSelection).Star1);
  settings.star1Resistance->num->SetValue(blades.at(lastBladeSelection).Star1Resistance);
  settings.star2Color->SetValue(blades.at(lastBladeSelection).Star2);
  settings.star2Resistance->num->SetValue(blades.at(lastBladeSelection).Star2Resistance);
  settings.star3Color->SetValue(blades.at(lastBladeSelection).Star3);
  settings.star3Resistance->num->SetValue(blades.at(lastBladeSelection).Star3Resistance);
  settings.star4Color->SetValue(blades.at(lastBladeSelection).Star4);
  settings.star4Resistance->num->SetValue(blades.at(lastBladeSelection).Star4Resistance);

  settings.subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(lastBladeSelection).subBlades.size() ? blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).startPixel : 0);
  settings.subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)blades.at(lastBladeSelection).subBlades.size() ? blades.at(lastBladeSelection).subBlades.at(lastSubBladeSelection).endPixel : 0);
  settings.subBladeUseStride->SetValue(blades.at(lastBladeSelection).subBladeWithStride);
}
void BladesPage::setEnabled() {
  settings.removeBlade->Enable(settings.bladeSelect->GetCount() > 0 && BD_HASSELECTION);
  settings.removeSubBlade->Enable(settings.subBladeSelect->GetCount() > 0 && BD_SUBHASSELECTION);
  settings.addSubBlade->Enable(BD_ISPIXEL && BD_HASSELECTION);

  settings.bladeType->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin1->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin2->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin3->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin4->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin5->Enable(BD_HASSELECTION && BD_ISFIRST);
  settings.usePowerPin6->Enable(BD_HASSELECTION && BD_ISFIRST);
}
void BladesPage::setVisibility(){
  settings.bladeColorOrderLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  settings.blade3ColorOrder->Show(BD_ISPIXEL3 && BD_ISFIRST);
  settings.blade4ColorOrder->Show(BD_ISPIXEL4 && BD_ISFIRST);
  settings.blade4UseRGB->Show(BD_ISPIXEL4 && BD_ISFIRST);

  settings.bladeDataPinLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  settings.bladeDataPin->Show(BD_ISPIXEL && BD_ISFIRST);
  settings.bladePixelsLabel->Show(BD_ISPIXEL && BD_ISFIRST);
  settings.bladePixels->Show(BD_ISPIXEL && BD_ISFIRST);

  settings.star1ColorLabel->Show(BD_ISSTAR);
  settings.star1Color->Show(BD_ISSTAR);
  settings.star1Resistance->box->Show(BD_ISSTAR);
  settings.star1Resistance->num->Enable(settings.star1Color->GetStringSelection() != BD_NORESISTANCE);
  settings.star2ColorLabel->Show(BD_ISSTAR);
  settings.star2Color->Show(BD_ISSTAR);
  settings.star2Resistance->box->Show(BD_ISSTAR);
  settings.star2Resistance->num->Enable(settings.star2Color->GetStringSelection() != BD_NORESISTANCE);
  settings.star3ColorLabel->Show(BD_ISSTAR);
  settings.star3Color->Show(BD_ISSTAR);
  settings.star3Resistance->box->Show(BD_ISSTAR);
  settings.star3Resistance->num->Enable(settings.star3Color->GetStringSelection() != BD_NORESISTANCE);
  settings.star4ColorLabel->Show(BD_ISSTAR4);
  settings.star4Color->Show(BD_ISSTAR4);
  settings.star4Resistance->box->Show(BD_ISSTAR4);
  settings.star4Resistance->num->Enable(settings.star4Color->GetStringSelection() != BD_NORESISTANCE);

  settings.subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
  settings.subBladeStartLabel->Show(BD_SUBHASSELECTION);
  settings.subBladeStart->Show(BD_SUBHASSELECTION);
  settings.subBladeEndLabel->Show(BD_SUBHASSELECTION);
  settings.subBladeEnd->Show(BD_SUBHASSELECTION);
}

void BladesPage::addBlade() {
  saveCurrent();

  blades.push_back(bladeConfig());
  lastBladeSelection = blades.size() - 1;

  update();
}
void BladesPage::addSubBlade() {
  saveCurrent();

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
