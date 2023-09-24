#include "bladespage.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "misc.h"
#include "defines.h"
#include "configuration.h"

BladesPage::BladesPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "Blades")
{
    wxBoxSizer *bladeManager = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bladeSelection = new wxBoxSizer(wxVERTICAL);
    wxStaticText *bladeText = new wxStaticText(GetStaticBox(), wxID_ANY, "Blades");
    settings.bladeSelect = new wxListBox(GetStaticBox(), Misc::ID_BladeSelect);
    wxBoxSizer *bladeButtons = new wxBoxSizer(wxHORIZONTAL);
    settings.addBlade = new wxButton(GetStaticBox(), Misc::ID_AddBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    settings.removeBlade = new wxButton(GetStaticBox(), Misc::ID_RemoveBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    bladeButtons->Add(settings.addBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
    bladeButtons->Add(settings.removeBlade, wxSizerFlags(0).Border(wxTOP, 10));
    bladeSelection->Add(bladeText, wxSizerFlags(0));
    bladeSelection->Add(settings.bladeSelect, wxSizerFlags(1).Expand());
    bladeSelection->Add(bladeButtons, wxSizerFlags(0));

    wxBoxSizer *subBladeSelection = new wxBoxSizer(wxVERTICAL);
    wxStaticText *subBladeText = new wxStaticText(GetStaticBox(), wxID_ANY, "SubBlades");
    settings.subBladeSelect = new wxListBox(GetStaticBox(), Misc::ID_SubBladeSelect);
    wxBoxSizer *subBladeButtons = new wxBoxSizer(wxHORIZONTAL);
    settings.addSubBlade = new wxButton(GetStaticBox(), Misc::ID_AddSubBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    settings.removeSubBlade = new wxButton(GetStaticBox(), Misc::ID_RemoveSubBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    subBladeButtons->Add(settings.addSubBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
    subBladeButtons->Add(settings.removeSubBlade, wxSizerFlags(0).Border(wxTOP, 10));
    subBladeSelection->Add(subBladeText, wxSizerFlags(0));
    subBladeSelection->Add(settings.subBladeSelect, wxSizerFlags(1).Expand());
    subBladeSelection->Add(subBladeButtons, wxSizerFlags(0));

    bladeManager->Add(bladeSelection, wxSizerFlags(0).Expand());
    bladeManager->Add(subBladeSelection, wxSizerFlags(0).Expand());


    wxWrapSizer *bladeSetup = new wxWrapSizer(wxVERTICAL);
    settings.bladeType = new wxComboBox(GetStaticBox(), Misc::ID_BladeType, "NeoPixel (RGB)", wxDefaultPosition, wxDefaultSize, {"NeoPixel (RGB)", "NeoPixel (RGBW)", "Tri-Star Cree", "Quad-Star Cree", "Single Color"}, wxCB_READONLY);
    settings.usePowerPin1 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 1");
    settings.usePowerPin2 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 2");
    settings.usePowerPin3 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 3");
    settings.usePowerPin4 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 4");
    settings.usePowerPin5 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 5");
    settings.usePowerPin6 = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Power Pin 6");
    bladeSetup->Add(settings.bladeType, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin1, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin2, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin3, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin4, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin5, MENUITEMFLAGS);
    bladeSetup->Add(settings.usePowerPin6, MENUITEMFLAGS);

    wxWrapSizer *bladeSettings = new wxWrapSizer(wxVERTICAL);

    wxBoxSizer *bladeColor = new wxBoxSizer(wxVERTICAL);
    settings.bladeColorOrderLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Color Order");
    settings.blade3ColorOrder = new wxComboBox(GetStaticBox(), wxID_ANY, "GRB", wxDefaultPosition, wxDefaultSize, {"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}, wxCB_READONLY);
    settings.blade4ColorOrder = new wxComboBox(GetStaticBox(), wxID_ANY, "GRBW", wxDefaultPosition, wxDefaultSize, {"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}, wxCB_READONLY);
    bladeColor->Add(settings.bladeColorOrderLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeColor->Add(settings.blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeColor->Add(settings.blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    settings.blade4UseRGB = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use RGB with White");
    settings.bladeDataPinLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Blade Data Pin");
    settings.bladeDataPin = new wxComboBox(GetStaticBox(), wxID_ANY, "Pin 1", wxDefaultPosition, wxDefaultSize, {"Pin 1", "Pin 2", "Pin 3", "Pin 4"});
    settings.bladePixelsLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Number of Pixels");
    settings.bladePixels = new wxSpinCtrl(GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

    wxBoxSizer *star1Color = new wxBoxSizer(wxVERTICAL);
    settings.star1ColorLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Star 1 Color");
    settings.star1Color = new wxComboBox(GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    star1Color->Add(settings.star1ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    star1Color->Add(settings.star1Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    wxBoxSizer *star2Color = new wxBoxSizer(wxVERTICAL);
    settings.star2ColorLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Star 2 Color");
    settings.star2Color = new wxComboBox(GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    star2Color->Add(settings.star2ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    star2Color->Add(settings.star2Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    wxBoxSizer *star3Color = new wxBoxSizer(wxVERTICAL);
    settings.star3ColorLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Star 3 Color");
    settings.star3Color = new wxComboBox(GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    star3Color->Add(settings.star3ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    star3Color->Add(settings.star3Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    wxBoxSizer *star4Color = new wxBoxSizer(wxVERTICAL);
    settings.star4ColorLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Star 4 Color");
    settings.star4Color = new wxComboBox(GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    star4Color->Add(settings.star4ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    star4Color->Add(settings.star4Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    settings.subBladeUseStride = new wxCheckBox(GetStaticBox(), wxID_ANY, "Use Stride for SubBlade");
    settings.subBladeStartLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "SubBlade Start");
    settings.subBladeStart = new wxSpinCtrl(GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
    settings.subBladeEndLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "SubBlade End");
    settings.subBladeEnd = new wxSpinCtrl(GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
    //settings.bladeID

    bladeSettings->Add(bladeColor);
    bladeSettings->Add(settings.blade4UseRGB, MENUITEMFLAGS);
    bladeSettings->Add(star1Color);
    bladeSettings->Add(star2Color);
    bladeSettings->Add(star3Color);
    bladeSettings->Add(star4Color);
    bladeSettings->Add(settings.bladeDataPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladePixelsLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladePixels, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    bladeSettings->Add(settings.subBladeUseStride, MENUITEMFLAGS);
    bladeSettings->Add(settings.subBladeStartLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeStart, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeEndLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeEnd, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    Add(bladeManager, MENUITEMFLAGS);
    Add(bladeSetup, wxSizerFlags(0));
    Add(bladeSettings, wxSizerFlags(1));


}

void BladesPage::update() {
    // Set Values for next Run (referenced here)
    lastBladeSelection = settings.bladeSelect->GetSelection();
    lastSubBladeSelection = settings.subBladeSelect->GetSelection();

    // Rebuild/Populate Blades
    settings.bladeSelect->Clear();
    for (int32_t bladeNum = 0; bladeNum < (int32_t)Configuration::blades.size(); bladeNum++) {
        settings.bladeSelect->Append(wxString::FromUTF8("Blade " + std::to_string(bladeNum)));
    }
    if ((int32_t)settings.bladeSelect->GetCount() - 1 >= lastBladeSelection) settings.bladeSelect->SetSelection(lastBladeSelection);

    // Rebuild/Populate SubBlades
    settings.subBladeSelect->Clear();
    for (int32_t subBladeNum = 0; lastBladeSelection != -1 && subBladeNum < (int32_t)Configuration::blades[lastBladeSelection].subBlades.size(); subBladeNum++) {
        settings.subBladeSelect->Append(wxString::FromUTF8("SubBlade " + std::to_string(subBladeNum)));
    }
    if ((int32_t)settings.subBladeSelect->GetCount() - 1 >= lastSubBladeSelection) settings.subBladeSelect->SetSelection(lastSubBladeSelection);

    // Recall Options
    if (Configuration::blades.size() > 0 && lastBladeSelection >= 0 && lastBladeSelection < (int32_t)Configuration::blades.size()) {
        settings.bladeType->SetValue(Configuration::blades[lastBladeSelection].type);
        settings.usePowerPin1->SetValue(Configuration::blades[lastBladeSelection].usePowerPin1);
        settings.usePowerPin2->SetValue(Configuration::blades[lastBladeSelection].usePowerPin2);
        settings.usePowerPin3->SetValue(Configuration::blades[lastBladeSelection].usePowerPin3);
        settings.usePowerPin4->SetValue(Configuration::blades[lastBladeSelection].usePowerPin4);
        settings.usePowerPin5->SetValue(Configuration::blades[lastBladeSelection].usePowerPin5);
        settings.usePowerPin6->SetValue(Configuration::blades[lastBladeSelection].usePowerPin6);

        settings.bladeDataPin->SetValue(Configuration::blades[lastBladeSelection].dataPin);
        settings.bladePixels->SetValue(Configuration::blades[lastBladeSelection].numPixels);
        settings.blade3ColorOrder->SetValue(Configuration::blades[lastBladeSelection].colorType);
        settings.blade4ColorOrder->SetValue(Configuration::blades[lastBladeSelection].colorType);
        settings.blade4UseRGB->SetValue(Configuration::blades[lastBladeSelection].useRGBWithWhite);

        settings.star1Color->SetValue(Configuration::blades[lastBladeSelection].Cree1);
        settings.star2Color->SetValue(Configuration::blades[lastBladeSelection].Cree2);
        settings.star3Color->SetValue(Configuration::blades[lastBladeSelection].Cree3);
        settings.star4Color->SetValue(Configuration::blades[lastBladeSelection].Cree4);

        settings.subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)Configuration::blades[lastBladeSelection].subBlades.size() ? Configuration::blades[lastBladeSelection].subBlades[lastSubBladeSelection].startPixel : 0);
        settings.subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int32_t)Configuration::blades[lastBladeSelection].subBlades.size() ? Configuration::blades[lastBladeSelection].subBlades[lastSubBladeSelection].endPixel : 0);
        settings.subBladeUseStride->SetValue(Configuration::blades[lastBladeSelection].subBladeWithStride);
    }

    // Enable/Disable Elements
    settings.removeBlade->Enable(settings.bladeSelect->GetCount() > 0 && BD_HASSELECTION);
    settings.removeSubBlade->Enable(settings.subBladeSelect->GetCount() > 0 && BD_SUBHASSELECTION);
    settings.addSubBlade->Enable(BD_ISNEOPIXEL && BD_HASSELECTION);

    settings.bladeType->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin1->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin2->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin3->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin4->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin5->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin6->Enable(BD_HASSELECTION && BD_ISFIRST);

    // Show/Unshow Elements
    settings.bladeColorOrderLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.blade3ColorOrder->Show(BD_ISNEOPIXEL3 && BD_ISFIRST);
    settings.blade4ColorOrder->Show(BD_ISNEOPIXEL4 && BD_ISFIRST);
    settings.blade4UseRGB->Show(BD_ISNEOPIXEL4 && BD_ISFIRST);

    settings.bladeDataPinLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladeDataPin->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladePixelsLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladePixels->Show(BD_ISNEOPIXEL && BD_ISFIRST);

    settings.star1ColorLabel->Show(BD_ISCREE);
    settings.star1Color->Show(BD_ISCREE);
    settings.star2ColorLabel->Show(BD_ISCREE);
    settings.star2Color->Show(BD_ISCREE);
    settings.star3ColorLabel->Show(BD_ISCREE);
    settings.star3Color->Show(BD_ISCREE);
    settings.star4ColorLabel->Show(BD_ISCREE4);
    settings.star4Color->Show(BD_ISCREE4);

    settings.subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
    settings.subBladeStartLabel->Show(BD_SUBHASSELECTION);
    settings.subBladeStart->Show(BD_SUBHASSELECTION);
    settings.subBladeEndLabel->Show(BD_SUBHASSELECTION);
    settings.subBladeEnd->Show(BD_SUBHASSELECTION);
}

decltype(BladesPage::settings) BladesPage::settings;

int32_t BladesPage::lastBladeSelection = -1;
int32_t BladesPage::lastSubBladeSelection = -1;
