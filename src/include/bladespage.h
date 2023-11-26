#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include "misc.h"

#pragma once

#define BD_PIXELRGB "WS281X (RGB)"
#define BD_PIXELRGBW "WS281X (RGBW)"
#define BD_TRISTAR "Tri-LED Star"
#define BD_QUADSTAR "Quad-LED Star"
#define BD_SINGLELED "Single Color"
#define BD_NORESISTANCE "<None>"

#define BD_HASSELECTION (bladeSelect->GetSelection() != -1)
#define BD_SUBHASSELECTION (subBladeSelect->GetSelection() != -1)
#define BD_ISPIXEL3 (BD_HASSELECTION && blades[bladeSelect->GetSelection()].type == BD_PIXELRGB)
#define BD_ISPIXEL4 (BD_HASSELECTION && blades[bladeSelect->GetSelection()].type == BD_PIXELRGBW)
#define BD_ISPIXEL (BD_ISPIXEL3 || BD_ISPIXEL4)
#define BD_ISSTAR3 (BD_HASSELECTION && blades[bladeSelect->GetSelection()].type == BD_TRISTAR)
#define BD_ISSTAR4 (BD_HASSELECTION && blades[bladeSelect->GetSelection()].type == BD_QUADSTAR)
#define BD_ISSTAR (BD_ISSTAR3 || BD_ISSTAR4)
#define BD_ISSUB (BD_HASSELECTION && blades[bladeSelect->GetSelection()].isSubBlade)
#define BD_ISFIRST (!BD_ISSUB || (subBladeSelect->GetSelection() == 0))

class BladesPage : public wxStaticBoxSizer
{
public:
  BladesPage(wxWindow*);
  static BladesPage* instance;

  void update();

  void addBlade();
  void addSubBlade();
  void removeBlade();
  void removeSubBlade();

  wxComboBox* bladeArray{nullptr};
  wxListBox* bladeSelect{nullptr};
  wxListBox* subBladeSelect{nullptr};
  wxButton* addBladeButton{nullptr};
  wxButton* removeBladeButton{nullptr};
  wxButton* addSubBladeButton{nullptr};
  wxButton* removeSubBladeButton{nullptr};

  wxComboBox* bladeType{nullptr};
  wxStaticText* bladeDataPinLabel{nullptr};
  wxComboBox* bladeDataPin{nullptr};
  wxStaticText* bladePixelsLabel{nullptr};
  wxSpinCtrl* bladePixels{nullptr};

  wxCheckBox* usePowerPin1{nullptr};
  wxCheckBox* usePowerPin2{nullptr};
  wxCheckBox* usePowerPin3{nullptr};
  wxCheckBox* usePowerPin4{nullptr};
  wxCheckBox* usePowerPin5{nullptr};
  wxCheckBox* usePowerPin6{nullptr};

  wxStaticText* bladeColorOrderLabel{nullptr};
  wxComboBox* blade3ColorOrder{nullptr};
  wxComboBox* blade4ColorOrder{nullptr};
  wxCheckBox* blade4UseRGB{nullptr};
  wxStaticText* star1ColorLabel{nullptr};
  wxComboBox* star1Color{nullptr};
  Misc::numEntry* star1Resistance{nullptr};
  wxStaticText* star2ColorLabel{nullptr};
  wxComboBox* star2Color{nullptr};
  Misc::numEntry* star2Resistance{nullptr};
  wxStaticText* star3ColorLabel{nullptr};
  wxComboBox* star3Color{nullptr};
  Misc::numEntry* star3Resistance{nullptr};
  wxStaticText* star4ColorLabel{nullptr};
  wxComboBox* star4Color{nullptr};
  Misc::numEntry* star4Resistance{nullptr};

  wxCheckBox* subBladeUseStride{nullptr};
  wxStaticText* subBladeStartLabel{nullptr};
  wxSpinCtrl* subBladeStart{nullptr};
  wxStaticText* subBladeEndLabel{nullptr};
  wxSpinCtrl* subBladeEnd{nullptr};

  struct bladeConfig {
    wxString type{BD_PIXELRGB};

    wxString dataPin{"bladePin"};
    wxString colorType{"GRB"};
    int32_t numPixels{0};
    bool useRGBWithWhite{false};

    wxString Star1{BD_NORESISTANCE};
    wxString Star2{BD_NORESISTANCE};
    wxString Star3{BD_NORESISTANCE};
    wxString Star4{BD_NORESISTANCE};
    int32_t Star1Resistance{0};
    int32_t Star2Resistance{0};
    int32_t Star3Resistance{0};
    int32_t Star4Resistance{0};

    bool usePowerPin1{false};
    bool usePowerPin2{false};
    bool usePowerPin3{false};
    bool usePowerPin4{false};
    bool usePowerPin5{false};
    bool usePowerPin6{false};

    bool isSubBlade{false};
    bool subBladeWithStride{false};

    struct subBladeInfo {
      int32_t startPixel{0};
      int32_t endPixel{0};
    };
    std::vector<subBladeInfo> subBlades{};
  };
  std::vector<bladeConfig> blades;

  enum class STARTYPE {
    RED,
    GREEN,
    PCAMBER,
    AMBER,
    BLUE,
    REDORANGE,
    WHITE,
    XPL,
    NOLED
  };
  enum class BLADETYPE {
    PIXEL_3,
    PIXEL_4,
    STAR_3,
    STAR_4,
    SINGLECOLOR,
    NONE
  };
  enum class C_ORDER {
    BGR,
    BRG,
    GBR,
    GRB,
    RBG,
    RGB,
    BGRW,
    BRGW,
    GBRW,
    GRBW,
    RBGW,
    RGBW,
    WBGR,
    WBRG,
    WGBR,
    WGRB,
    WRBG,
    WRGB,
    BGRw,
    BRGw,
    GBRw,
    GRBw,
    RBGw,
    RGBw,
    wBGR,
    wBRG,
    wGBR,
    wGRB,
    wRBG,
    wRGB
  };


private:
  BladesPage();

  wxBoxSizer* createBladeManager();

  void saveCurrent();
  void rebuildBladeArray();
  void loadSettings();
  void setEnabled();
  void setVisibility();

  int lastBladeSelection{-1};
  int lastSubBladeSelection{-1};
};
