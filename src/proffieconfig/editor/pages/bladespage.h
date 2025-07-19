#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024-2025 Ryan Ogurek

#include "../editorwindow.h"

#include "ui/controls/checklist.h"
#include "ui/controls/combobox.h"
#include "ui/controls/numeric.h"
#include "ui/controls/radios.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"

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

    // BladeArrayDlg *bladeArrayDlg{nullptr};

    // wxButton *bladeArrayButton{nullptr};
    PCUI::Choice *bladeArray{nullptr};
    PCUI::List *bladeSelect{nullptr};
    PCUI::List *subBladeSelect{nullptr};

    PCUI::Choice *bladeType{nullptr};
    PCUI::ComboBox *bladeDataPin{nullptr};
    PCUI::Numeric *bladePixels{nullptr};

    PCUI::CheckList *powerPins{nullptr};
    PCUI::Text *powerPinName{nullptr};

    PCUI::Choice *blade3ColorOrder{nullptr};
    PCUI::Choice *blade4ColorOrder{nullptr};
    PCUI::CheckBox *blade4UseRGB{nullptr};

    struct StarUI {
        PCUI::Choice *led{nullptr};
        PCUI::Numeric *resistance{nullptr};
        PCUI::ComboBox *powerPin{nullptr};
    };

    StarUI star1;
    StarUI star2;
    StarUI star3;
    StarUI star4;

    PCUI::Radios *subBladeType{nullptr};
    PCUI::Numeric *subBladeLength{nullptr};
    PCUI::Numeric *subBladeSegments{nullptr};

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
};
