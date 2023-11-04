#include "bladespage.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "misc.h"
#include "defines.h"
#include "configuration.h"

BladesPage* BladesPage::instance;
BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
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
  settings.bladeType = new wxComboBox( GetStaticBox(), Misc::ID_BladeType, "WS281X (RGB)", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"WS281X (RGB)", "WS281X (RGBW)", "Tri-LED Star", "Quad-LED Star", "Single Color"}), wxCB_READONLY);
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
  settings.star1Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, "<None>", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}), wxCB_READONLY);
  settings.star1Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star1->Add(settings.star1ColorLabel, MENUITEMFLAGS);
  star1->Add(settings.star1Color, MENUITEMFLAGS);
  star1->Add(settings.star1Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star2 = new wxBoxSizer( wxVERTICAL);
  settings.star2ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 2 Color");
  settings.star2Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, "<None>", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}), wxCB_READONLY);
  settings.star2Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star2->Add(settings.star2ColorLabel, MENUITEMFLAGS);
  star2->Add(settings.star2Color, MENUITEMFLAGS);
  star2->Add(settings.star2Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star3 = new wxBoxSizer( wxVERTICAL);
  settings.star3ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 3 Color");
  settings.star3Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, "<None>", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}), wxCB_READONLY);
  settings.star3Resistance = Misc::createNumEntry(this, "Resistance", wxID_ANY, 0, 10000, 500);
  star3->Add(settings.star3ColorLabel, MENUITEMFLAGS);
  star3->Add(settings.star3Color, MENUITEMFLAGS);
  star3->Add(settings.star3Resistance->box, MENUITEMFLAGS);

  wxBoxSizer* star4 = new wxBoxSizer( wxVERTICAL);
  settings.star4ColorLabel = new wxStaticText( GetStaticBox(), wxID_ANY, "LED 4 Color");
  settings.star4Color = new wxComboBox( GetStaticBox(), Misc::ID_BladeOption, "<None>", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}), wxCB_READONLY);
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

  update();
}

void BladesPage::update() {
  // Set Values for next Run (referenced here)
  lastBladeSelection = settings.bladeSelect->GetSelection();
  lastSubBladeSelection = settings.subBladeSelect->GetSelection();

  if (Configuration::instance->blades.size() == 0) Configuration::instance->blades.push_back(Configuration::bladeConfig());

  // Rebuild/Populate Blades
  settings.bladeSelect->Clear();
  for (int32_t bladeNum = 0; bladeNum < (int32_t)Configuration::instance->blades.size(); bladeNum++) {
    settings.bladeSelect->Append("Blade " + std::to_string(bladeNum));
  }
  if ((int32_t)settings.bladeSelect->GetCount() - 1 >= lastBladeSelection) settings.bladeSelect->SetSelection(lastBladeSelection);

  // Rebuild/Populate SubBlades
  settings.subBladeSelect->Clear();
  for (int32_t subBladeNum = 0; lastBladeSelection != -1 && subBladeNum < (int32_t)Configuration::instance->blades[lastBladeSelection].subBlades.size(); subBladeNum++) {
    settings.subBladeSelect->Append("SubBlade " + std::to_string(subBladeNum));
  }
  if ((int32_t)settings.subBladeSelect->GetCount() - 1 >= lastSubBladeSelection) settings.subBladeSelect->SetSelection(lastSubBladeSelection);

  // Recall Options
  if (Configuration::instance->blades.size() > 0 && lastBladeSelection >= 0 && lastBladeSelection < (int32_t)Configuration::instance->blades.size()) {
    settings.bladeType->ChangeValue(Configuration::instance->blades[lastBladeSelection].type);
    settings.usePowerPin1->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin1);
    settings.usePowerPin2->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin2);
    settings.usePowerPin3->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin3);
    settings.usePowerPin4->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin4);
    settings.usePowerPin5->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin5);
    settings.usePowerPin6->SetValue(Configuration::instance->blades[lastBladeSelection].usePowerPin6);

    settings.bladeDataPin->ChangeValue(Configuration::instance->blades[lastBladeSelection].dataPin);
    settings.bladePixels->SetValue(Configuration::instance->blades[lastBladeSelection].numPixels);
    settings.blade3ColorOrder->ChangeValue(Configuration::instance->blades[lastBladeSelection].colorType);
    settings.blade4ColorOrder->ChangeValue(Configuration::instance->blades[lastBladeSelection].colorType);
    settings.blade4UseRGB->SetValue(Configuration::instance->blades[lastBladeSelection].useRGBWithWhite);

    settings.star1Color->ChangeValue(Configuration::instance->blades[lastBladeSelection].Star1);
    settings.star1Resistance->num->SetValue(Configuration::instance->blades[lastBladeSelection].Star1Resistance);
    settings.star2Color->ChangeValue(Configuration::instance->blades[lastBladeSelection].Star2);
    settings.star2Resistance->num->SetValue(Configuration::instance->blades[lastBladeSelection].Star2Resistance);
    settings.star3Color->ChangeValue(Configuration::instance->blades[lastBladeSelection].Star3);
    settings.star3Resistance->num->SetValue(Configuration::instance->blades[lastBladeSelection].Star3Resistance);
    settings.star4Color->ChangeValue(Configuration::instance->blades[lastBladeSelection].Star4);
    settings.star4Resistance->num->SetValue(Configuration::instance->blades[lastBladeSelection].Star4Resistance);

    settings.subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)Configuration::instance->blades[lastBladeSelection].subBlades.size() ? Configuration::instance->blades[lastBladeSelection].subBlades[lastSubBladeSelection].startPixel : 0);
    settings.subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)Configuration::instance->blades[lastBladeSelection].subBlades.size() ? Configuration::instance->blades[lastBladeSelection].subBlades[lastSubBladeSelection].endPixel : 0);
    settings.subBladeUseStride->SetValue(Configuration::instance->blades[lastBladeSelection].subBladeWithStride);
  }

  // Enable/Disable Elements
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

  // Show/Unshow Elements
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
  settings.star1Resistance->num->Enable(settings.star1Color->GetStringSelection() != "<None>");
  settings.star2ColorLabel->Show(BD_ISSTAR);
  settings.star2Color->Show(BD_ISSTAR);
  settings.star2Resistance->box->Show(BD_ISSTAR);
  settings.star2Resistance->num->Enable(settings.star2Color->GetStringSelection() != "<None>");
  settings.star3ColorLabel->Show(BD_ISSTAR);
  settings.star3Color->Show(BD_ISSTAR);
  settings.star3Resistance->box->Show(BD_ISSTAR);
  settings.star3Resistance->num->Enable(settings.star3Color->GetStringSelection() != "<None>");
  settings.star4ColorLabel->Show(BD_ISSTAR4);
  settings.star4Color->Show(BD_ISSTAR4);
  settings.star4Resistance->box->Show(BD_ISSTAR4);
  settings.star4Resistance->num->Enable(settings.star4Color->GetStringSelection() != "<None>");

  settings.subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
  settings.subBladeStartLabel->Show(BD_SUBHASSELECTION);
  settings.subBladeStart->Show(BD_SUBHASSELECTION);
  settings.subBladeEndLabel->Show(BD_SUBHASSELECTION);
  settings.subBladeEnd->Show(BD_SUBHASSELECTION);
}

void BladesPage::addBlade() {
  Configuration::instance->blades.push_back(Configuration::Configuration::bladeConfig());
  BladesPage::instance->lastBladeSelection = Configuration::instance->blades.size() - 1;

  BladesPage::instance->update();
}
void BladesPage::addSubBlade() {
  Configuration::instance->blades[BladesPage::instance->lastBladeSelection].isSubBlade = true;
  Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.push_back(Configuration::bladeConfig::subBladeInfo());
  if (Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.size() <= 1) Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.push_back(Configuration::bladeConfig::subBladeInfo());
  Configuration::instance->updateBladesConfig();
  BladesPage::instance->update();
}
void BladesPage::removeBlade() {
  if (BD_HASSELECTION && Configuration::instance->blades.size() > 1) {
    Configuration::instance->blades.erase(Configuration::instance->blades.begin() + BladesPage::instance->lastBladeSelection);
  }
  Configuration::instance->updateBladesConfig();
  BladesPage::instance->update();
}
void BladesPage::removeSubBlade() {
  if (BD_SUBHASSELECTION) {
    Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.erase(Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.begin() + BladesPage::instance->lastSubBladeSelection);
    if (Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.size() <= 1) {
      Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.clear();
      Configuration::instance->blades[BladesPage::instance->lastBladeSelection].isSubBlade = false;
    }
    BladesPage::instance->lastSubBladeSelection = -1;
  }
  Configuration::instance->updateBladesConfig();
  BladesPage::instance->update();
}
