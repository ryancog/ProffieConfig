#pragma once
#include <wx/event.h>

#define FIRSTITEMFLAGS wxSizerFlags(0).Border(wxALL, 5)
#define MENUITEMFLAGS  wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5)
#define BOXITEMFLAGS wxSizerFlags(0).Border(wxALL, 10).Expand()

#define UPDATEWINDOW master->Layout(); SetSizerAndFit(master)


#define BD_HASSELECTION (BladesPage::instance->settings.bladeSelect->GetSelection() != -1)
#define BD_SUBHASSELECTION (BladesPage::instance->settings.subBladeSelect->GetSelection() != -1)
#define BD_ISNEOPIXEL3 (BD_HASSELECTION && Configuration::instance->blades[BladesPage::instance->settings.bladeSelect->GetSelection()].type == "NeoPixel (RGB)")
#define BD_ISNEOPIXEL4 (BD_HASSELECTION && Configuration::instance->blades[BladesPage::instance->settings.bladeSelect->GetSelection()].type == "NeoPixel (RGBW)")
#define BD_ISNEOPIXEL (BD_ISNEOPIXEL3 || BD_ISNEOPIXEL4)
#define BD_ISCREE3 (BD_HASSELECTION && Configuration::instance->blades[BladesPage::instance->settings.bladeSelect->GetSelection()].type == "Tri-Star Cree")
#define BD_ISCREE4 (BD_HASSELECTION && Configuration::instance->blades[BladesPage::instance->settings.bladeSelect->GetSelection()].type == "Quad-Star Cree")
#define BD_ISCREE (BD_ISCREE3 || BD_ISCREE4)
#define BD_ISSUB (BD_HASSELECTION && Configuration::instance->blades[BladesPage::instance->settings.bladeSelect->GetSelection()].isSubBlade)
#define BD_ISFIRST (!BD_ISSUB || (BladesPage::instance->settings.subBladeSelect->GetSelection() == 0))

#define PR_DEFAULT "Default"
#define PR_SA22C "SA22C"
#define PR_FETT263 "Fett263"
#define PR_SHTOK "Shtok"
#define PR_BC "BC"
#define PR_CAIWYN "Caiwyn"
