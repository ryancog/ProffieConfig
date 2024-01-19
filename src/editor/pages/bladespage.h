// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek


#pragma once

#include "ui/pcspinctrl.h"
#include "ui/pctextctrl.h"
#include "editor/editorwindow.h"

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

#define BD_PIXELRGB "WS281X (RGB)"
#define BD_PIXELRGBW "WS281X (RGBW)"
#define BD_TRISTAR "Tri-LED Star"
#define BD_QUADSTAR "Quad-LED Star"
#define BD_SINGLELED "Single Color"
#define BD_NORESISTANCE "<None>"

#define BD_HASSELECTION (bladeSelect->GetSelection() != -1)
#define BD_SUBHASSELECTION (subBladeSelect->GetSelection() != -1)
#define BD_ISPIXEL3 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGB)
#define BD_ISPIXEL4 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGBW)
#define BD_ISPIXEL (BD_ISPIXEL3 || BD_ISPIXEL4)
#define BD_ISSTAR3 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_TRISTAR)
#define BD_ISSTAR4 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_QUADSTAR)
#define BD_ISSTAR (BD_ISSTAR3 || BD_ISSTAR4)
#define BD_ISSUB (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].isSubBlade)
#define BD_ISFIRST (!BD_ISSUB || (subBladeSelect->GetSelection() == 0))
#define BD_ISSTNDRD (BD_ISSUB && useStandard->GetValue())
#define BD_ISSTRIDE (BD_ISSUB && useStride->GetValue())
#define BD_ISZIGZAG (BD_ISSUB && useZigZag->GetValue())

class BladesPage : public wxStaticBoxSizer {
public:
  BladesPage(wxWindow*);

  void update();

  void addBlade();
  void addSubBlade();
  void removeBlade();
  void removeSubBlade();

  int32_t lastBladeArraySelection{0};
  
  BladeArrayDlg* bladeArrayDlg{nullptr};

  wxButton* bladeArrayButton{nullptr};
  pcComboBox* bladeArray{nullptr};
  wxListBox* bladeSelect{nullptr};
  wxListBox* subBladeSelect{nullptr};
  wxButton* addBladeButton{nullptr};
  wxButton* removeBladeButton{nullptr};
  wxButton* addSubBladeButton{nullptr};
  wxButton* removeSubBladeButton{nullptr};

  pcComboBox* bladeType{nullptr};
  pcComboBox* bladeDataPin{nullptr};
  wxStaticText* bladePixelsLabel{nullptr};
  pcSpinCtrl* bladePixels{nullptr};

  wxCheckListBox* powerPins{nullptr};
  wxButton* addPowerPin{nullptr};
  pcTextCtrl* powerPinName{nullptr};

  pcComboBox* blade3ColorOrder{nullptr};
  pcComboBox* blade4ColorOrder{nullptr};
  wxCheckBox* blade4UseRGB{nullptr};
  pcComboBox* star1Color{nullptr};
  pcSpinCtrl* star1Resistance{nullptr};
  pcComboBox* star2Color{nullptr};
  pcSpinCtrl* star2Resistance{nullptr};
  pcComboBox* star3Color{nullptr};
  pcSpinCtrl* star3Resistance{nullptr};
  pcComboBox* star4Color{nullptr};
  pcSpinCtrl* star4Resistance{nullptr};

  wxRadioButton* useStandard{nullptr};
  wxRadioButton* useStride{nullptr};
  wxRadioButton* useZigZag{nullptr};
  pcSpinCtrl* subBladeStart{nullptr};
  pcSpinCtrl* subBladeEnd{nullptr};

  enum {
    ID_BladeArray,
    ID_OpenBladeArrays,
    ID_BladeSelect,
    ID_SubBladeSelect,
    ID_BladeType,
    ID_AddBlade,
    ID_AddSubBlade,
    ID_RemoveBlade,
    ID_RemoveSubBlade,

    ID_PowerPins,
    ID_AddPowerPin,
    ID_PowerPinName,
  };

  struct BladeConfig {
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

    std::vector<std::string> powerPins;

    bool isSubBlade{false};
    bool useStride{false};
    bool useZigZag{false};

    struct subBladeInfo {
      uint32_t startPixel{0};
      uint32_t endPixel{0};
    };
    std::vector<subBladeInfo> subBlades{};
  };

private:
  EditorWindow* parent{nullptr};

  void bindEvents();
  void createToolTips();

  wxBoxSizer* createBladeSelect();
  wxBoxSizer* createBladeManager();
  wxBoxSizer* createBladeSetup();
  wxBoxSizer* createBladeSettings();

  void saveCurrent();
  void rebuildBladeArray();
  void loadSettings();
  void setEnabled();
  void setVisibility();
  void updateRanges();

  int32_t lastBladeSelection{-1};
  int32_t lastSubBladeSelection{-1};
};
