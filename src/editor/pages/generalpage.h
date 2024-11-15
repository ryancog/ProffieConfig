// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "editor/dialogs/customoptionsdlg.h"
#include "ui/pcspinctrl.h"
#include "ui/pcspinctrldouble.h"

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>

#include <array>

class GeneralPage : public wxStaticBoxSizer {
public:
    GeneralPage(EditorWindow*);

    pcChoice* board{nullptr};
    wxCheckBox* massStorage{nullptr};
    wxCheckBox* webUSB{nullptr};

    CustomOptionsDlg* customOptDlg{nullptr};
    wxButton* customOptButton{nullptr};

    pcChoice* orientation{nullptr};
    pcSpinCtrl* buttons{nullptr};
    pcSpinCtrl* volume{nullptr};
    pcSpinCtrlDouble* clash{nullptr};
    pcSpinCtrl* pliTime{nullptr};
    pcSpinCtrl* idleTime{nullptr};
    pcSpinCtrl* motionTime{nullptr};
    pcSpinCtrl* maxLEDs{nullptr};

    enum OPTION {
        SAVE_VOLUME,
        SAVE_PRESET,
        SAVE_COLOR,
        SAVE_CLASH_THRESHOLD,
        SAVE_BLADE_DIMMING,
        DYNAMIC_BLADE_LENGTH,
        DYNAMIC_BLADE_DIMMING,
        DYNAMIC_CLASH_THRESHOLD,

        ENABLE_OLED,

        DISABLE_COLOR_CHANGE,
        DISABLE_TALKIE,
        DISABLE_BASIC_PARSER_STYLES,
        DISABLE_DIAGNOSTIC_COMMANDS,

        OPTIONS_MAX,
    };
    static constexpr std::array<const char *, OPTIONS_MAX> OPTION_CONFIGSTRS {
        "SAVE_VOLUME",
        "SAVE_PRESET",
        "SAVE_COLOR_CHANGE",
        "SAVE_CLASH_THRESHOLD",
        "SAVE_BLADE_DIMMING",
        "DYNAMIC_BLADE_LENGTH",
        "DYNAMIC_BLADE_DIMMING",
        "DYNAMIC_CLASH_THRESHOLD",

        "ENABLE_SSD1306",

        "DISABLE_COLOR_CHANGE",
        "DISABLE_TALKIE",
        "DISABLE_BASIC_PARSER_STYLES",
        "DISABLE_DIAGNOSTIC_COMMANDS",
    };

    std::array<wxCheckBox *, OPTIONS_MAX> options { nullptr };

    enum {
        ID_CustomOptions,
    };

private:
    EditorWindow* parent{nullptr};

    void bindEvents();
    void createToolTips();

    wxStaticBoxSizer* boardSection(wxStaticBoxSizer*);
    wxStaticBoxSizer* optionSection(wxStaticBoxSizer*);
    wxBoxSizer* rightOptions(wxStaticBoxSizer*);
    wxBoxSizer* leftOptions(wxStaticBoxSizer*);
};
