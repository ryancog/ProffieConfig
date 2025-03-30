#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

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
#define BD_SIMPLE _("Simple LED")

class BladesPage : public wxStaticBoxSizer {
public:
    BladesPage(wxWindow*);

    void update();

    void addBlade();
    void addSubBlade();
    void removeBlade();
    void removeSubBlade();

    int32 lastBladeArraySelection{0};

    BladeArrayDlg *bladeArrayDlg{nullptr};

    wxButton *bladeArrayButton{nullptr};
    PCUI::Choice *bladeArray{nullptr};
    wxListBox *bladeSelect{nullptr};
    wxListBox *subBladeSelect{nullptr};
    wxButton *addBladeButton{nullptr};
    wxButton *removeBladeButton{nullptr};
    wxButton *addSubBladeButton{nullptr};
    wxButton *removeSubBladeButton{nullptr};

    PCUI::Choice *bladeType{nullptr};
    PCUI::ComboBox *bladeDataPin{nullptr};
    PCUI::Numeric *bladePixels{nullptr};

    wxCheckListBox *powerPins{nullptr};
    wxButton *addPowerPin{nullptr};
    PCUI::Text *powerPinName{nullptr};

    PCUI::Choice *blade3ColorOrder{nullptr};
    PCUI::Choice *blade4ColorOrder{nullptr};
    wxCheckBox *blade4UseRGB{nullptr};

    wxStaticBoxSizer *star1Sizer{nullptr};
    PCUI::Choice *star1Color{nullptr};
    PCUI::Numeric *star1Resistance{nullptr};
    wxStaticBoxSizer *star2Sizer{nullptr};
    PCUI::Choice *star2Color{nullptr};
    PCUI::Numeric *star2Resistance{nullptr};
    wxStaticBoxSizer *star3Sizer{nullptr};
    PCUI::Choice *star3Color{nullptr};
    PCUI::Numeric *star3Resistance{nullptr};
    wxStaticBoxSizer *star4Sizer{nullptr};
    PCUI::Choice *star4Color{nullptr};
    PCUI::Numeric *star4Resistance{nullptr};

    wxRadioButton *useStandard{nullptr};
    wxRadioButton *useStride{nullptr};
    wxRadioButton *useZigZag{nullptr};
    PCUI::Numeric *subBladeStart{nullptr};
    PCUI::Numeric *subBladeEnd{nullptr};

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

    enum LED : int32 {
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

    static wxArrayString ledStrings();
    static wxString ledToConfigStr(LED);
    static LED strToLed(const wxString&);
    static wxString ledToStr(LED);

    struct BladeConfig {
        wxString type{BD_PIXELRGB};

        wxString dataPin{"bladePin"};
        wxString colorType{"GRB"};
        int32 numPixels{0};
        bool useRGBWithWhite{false};

        LED star1{NONE};
        LED star2{NONE};
        LED star3{NONE};
        LED star4{NONE};
        int32 star1Resistance{0};
        int32 star2Resistance{0};
        int32 star3Resistance{0};
        int32 star4Resistance{0};

        vector<wxString> powerPins;

        bool isSubBlade{false};
        bool useStride{false};
        bool useZigZag{false};

        struct SubBladeInfo {
            SubBladeInfo(int32 startPixel, int32 endPixel) : startPixel{startPixel}, endPixel{endPixel} {}
            int32 startPixel{0};
            int32 endPixel{0};
        };
        vector<SubBladeInfo> subBlades;
    };

private:
    EditorWindow *mParent{nullptr};

    void bindEvents();
    void createToolTips() const;

    wxBoxSizer *createBladeSelect();
    wxBoxSizer *createBladeManager();
    wxBoxSizer *createBladeSetup();
    wxBoxSizer *createBladeSettings();

    void saveCurrent() const;
    void rebuildBladeArray();
    void loadSettings() const;
    void setEnabled();
    void setVisibility();
    void updateRanges();

    [[nodiscard]] bool hasBladeSelection() const;
    [[nodiscard]] bool hasSubBladeSelection() const;
    [[nodiscard]] bool isRGBPixelBlade() const;
    [[nodiscard]] bool isRGBWPixelBlade() const;
    [[nodiscard]] bool isPixelBlade() const;
    [[nodiscard]] bool isSimpleBlade() const;
    [[nodiscard]] bool isSubBlade() const;
    [[nodiscard]] bool isFirstBlade() const;
    [[nodiscard]] bool isStandardSubBlade() const;
    [[nodiscard]] bool isStrideSubBlade() const;
    [[nodiscard]] bool isZigZagSubBlade() const;

    int32 mLastBladeSelection{-1};
    int32 mLastSubBladeSelection{-1};
};
