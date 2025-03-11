#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../mainmenu/mainmenu.h"

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/filepicker.h>

class Props : public wxDialog {
public:
    Props(MainMenu*);

    enum {
        ID_AddProp,

        ID_Prop,
        ID_PropConfig,
    };

private:
    MainMenu* parent{nullptr};

    wxStaticText* choosePropText{nullptr};
    wxFilePickerCtrl* chooseProp{nullptr};
    wxStaticText* choosePropConfigText{nullptr};
    wxFilePickerCtrl* choosePropConfig{nullptr};

    wxStaticText* duplicateWarning{nullptr};
    wxStaticText* fileSelectionWarning{nullptr};

    void update();
    void createUI();
    void bindEvents();
};
