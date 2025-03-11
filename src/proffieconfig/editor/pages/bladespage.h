#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <array>

#include "ui/controls.h"
#include "../editorwindow.h"

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/radiobut.h>

#define BD_PIXELRGB "WS281X (RGB)"
#define BD_PIXELRGBW "WS281X (RGBW)"
#define BD_SIMPLE "Simple LED"

#define BD_HASSELECTION (bladeSelect->GetSelection() != -1)
#define BD_SUBHASSELECTION (subBladeSelect->GetSelection() != -1)
#define BD_ISPIXEL3 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGB)
#define BD_ISPIXEL4 (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_PIXELRGBW)
#define BD_ISPIXEL (BD_ISPIXEL3 || BD_ISPIXEL4)
#define BD_ISSIMPLE (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].type == BD_SIMPLE)
#define BD_ISSUB (BD_HASSELECTION && bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades[bladeSelect->GetSelection()].isSubBlade)
#define BD_ISFIRST (!BD_ISSUB || (subBladeSelect->GetSelection() == 0))
#define BD_ISSTNDRD (BD_ISSUB && useStandard->GetValue())
#define BD_ISSTRIDE (BD_ISSUB && useStride->GetValue())
#define BD_ISZIGZAG (BD_ISSUB && useZigZag->GetValue())

template<typename T, std::size_t... I1, std::size_t... I2>
constexpr auto concat_impl(const std::array<T, sizeof...(I1)>& a1, const std::array<T, sizeof...(I2)>& a2,
                           std::index_sequence<I1...>, std::index_sequence<I2...>) {
    return std::array<T, sizeof...(I1) + sizeof...(I2)>{ a1[I1]..., a2[I2]... };
}

template<typename T, std::size_t N1, std::size_t N2>
constexpr auto concat(const std::array<T, N1>& a1, const std::array<T, N2>& a2) {
    return concat_impl(a1, a2, std::make_index_sequence<N1>{}, std::make_index_sequence<N2>{});
}

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
    PCUI::Choice* bladeArray{nullptr};
    wxListBox* bladeSelect{nullptr};
    wxListBox* subBladeSelect{nullptr};
    wxButton* addBladeButton{nullptr};
    wxButton* removeBladeButton{nullptr};
    wxButton* addSubBladeButton{nullptr};
    wxButton* removeSubBladeButton{nullptr};

    PCUI::Choice* bladeType{nullptr};
    PCUI::ComboBox* bladeDataPin{nullptr};
    PCUI::Numeric* bladePixels{nullptr};

    wxCheckListBox* powerPins{nullptr};
    wxButton* addPowerPin{nullptr};
    PCUI::Text* powerPinName{nullptr};

    PCUI::Choice* blade3ColorOrder{nullptr};
    PCUI::Choice* blade4ColorOrder{nullptr};
    wxCheckBox* blade4UseRGB{nullptr};

    wxStaticBoxSizer *star1Sizer{nullptr};
    PCUI::Choice* star1Color{nullptr};
    PCUI::Numeric* star1Resistance{nullptr};
    wxStaticBoxSizer *star2Sizer{nullptr};
    PCUI::Choice* star2Color{nullptr};
    PCUI::Numeric* star2Resistance{nullptr};
    wxStaticBoxSizer *star3Sizer{nullptr};
    PCUI::Choice* star3Color{nullptr};
    PCUI::Numeric* star3Resistance{nullptr};
    wxStaticBoxSizer *star4Sizer{nullptr};
    PCUI::Choice* star4Color{nullptr};
    PCUI::Numeric* star4Resistance{nullptr};

    wxRadioButton* useStandard{nullptr};
    wxRadioButton* useStride{nullptr};
    wxRadioButton* useZigZag{nullptr};
    PCUI::Numeric* subBladeStart{nullptr};
    PCUI::Numeric* subBladeEnd{nullptr};

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

        ID_LEDColor
    };

    enum LED : int32_t {
        NONE = 0,

        CREE_RED          = 1 << 1,
        CREE_GREEN        = 1 << 2,
        CREE_BLUE         = 1 << 3,
        CREE_AMBER        = 1 << 4,
        CREE_RED_ORANGE   = 1 << 5,
        CREE_WHITE        = 1 << 6,
        CREE_LED = CREE_RED | CREE_GREEN | CREE_BLUE | CREE_AMBER | CREE_RED_ORANGE | CREE_WHITE,

        RED               = 1 << 7,
        GREEN             = 1 << 8,
        BLUE              = 1 << 9,
        SIMPLE_LED = RED | GREEN | BLUE,

        USES_RESISTANCE = CREE_LED,
    };

    static constexpr std::array<std::pair<LED, const char *>, 10> LED_STRINGS {{
        { NONE, "<None>" },
        { CREE_RED, "Cree Red" },
        { CREE_GREEN, "Cree Green" },
        { CREE_BLUE, "Cree Blue" },
        { CREE_AMBER, "Cree Amber" },
        { CREE_RED_ORANGE, "Cree Red-Orange" },
        { CREE_WHITE, "Cree White" },
        { RED, "Red" },
        { GREEN, "Green" },
        { BLUE, "Blue" },
    }};

    static constexpr std::array<std::pair<LED, const char *>, 10> LED_CONFIGSTRS {{
        { NONE, "NoLED" },
        { CREE_RED, "CreeXPE2RedTemplate" },
        { CREE_GREEN, "CreeXPE2GreenTemplate" },
        { CREE_BLUE, "CreeXPE2BlueTemplate" },
        { CREE_AMBER, "CreeXPE2AmberTemplate" },
        { CREE_RED_ORANGE, "CreeXPE2RedOrangeTemplate" },
        { CREE_WHITE, "CreeXPE2WhiteTemplate" },
        { RED, "CH1LED" },
        { GREEN, "CH2LED" },
        { BLUE, "CH3LED" },
    }};

    struct BladeConfig {
        wxString type{BD_PIXELRGB};

        wxString dataPin{"bladePin"};
        wxString colorType{"GRB"};
        int32_t numPixels{0};
        bool useRGBWithWhite{false};

        LED Star1{NONE};
        LED Star2{NONE};
        LED Star3{NONE};
        LED Star4{NONE};
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
