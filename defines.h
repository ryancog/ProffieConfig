#pragma once

#define MENUITEMDEFAULTFLAGS wxSizerFlags(0).Border(wxALL, 10).Expand()

#define UPDATEWINDOW master->Layout();


#define BD_HASSELECTION (settings.bladeSelect->GetSelection() != -1)
#define BD_SUBHASSELECTION (settings.subBladeSelect->GetSelection() != -1)
#define BD_ISNEOPIXEL3 (BD_HASSELECTION && config.blades[settings.bladeSelect->GetSelection()].type == "NeoPixel (RGB)")
#define BD_ISNEOPIXEL4 (BD_HASSELECTION && config.blades[settings.bladeSelect->GetSelection()].type == "NeoPixel (RGBW)")
#define BD_ISNEOPIXEL (BD_ISNEOPIXEL3 || BD_ISNEOPIXEL4)
#define BD_ISCREE3 (BD_HASSELECTION && config.blades[settings.bladeSelect->GetSelection()].type == "Tri-Star Cree")
#define BD_ISCREE4 (BD_HASSELECTION && config.blades[settings.bladeSelect->GetSelection()].type == "Quad-Star Cree")
#define BD_ISCREE (BD_ISCREE3 || BD_ISCREE4)
#define BD_ISSUB (BD_HASSELECTION && config.blades[settings.bladeSelect->GetSelection()].isSubBlade)
#define BD_ISFIRST (!BD_ISSUB || (settings.subBladeSelect->GetSelection() == 0))
