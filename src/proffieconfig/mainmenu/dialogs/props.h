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
        ID_ViewExisting,
        ID_AddProp,

        ID_Prop,
        ID_PropConfig,
    };

private:
    MainMenu* mParent{nullptr};

    wxStaticText* mChoosePropText{nullptr};
    wxFilePickerCtrl* mChooseProp{nullptr};
    wxStaticText* mChoosePropConfigText{nullptr};
    wxFilePickerCtrl* mChoosePropConfig{nullptr};

    wxStaticText* mDuplicateWarning{nullptr};
    wxStaticText* mFileSelectionWarning{nullptr};

    wxToggleButton *mViewExisting{nullptr};
    wxToggleButton *mAddProp{nullptr};

    wxPanel *mExistingPanel{nullptr};
    wxPanel *mAddPanel{nullptr};

    void update();
    void createUI();
    void bindEvents();
};
