#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../mainmenu/mainmenu.h"
#include "ui/controls.h"

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/filepicker.h>

class AddConfig : public wxDialog {
public:
    AddConfig(MainMenu *);
    enum {
        ID_CreateNew,
        ID_ImportExisting,
    };

    MainMenu* parent{nullptr};

    PCUI::FilePickerData importPath;
    PCUI::TextData configName;
    void update();

private:
    wxStaticText* mInvalidNameWarning{nullptr};
    wxStaticText* mDuplicateWarning{nullptr};
    wxStaticText* mFileSelectionWarning{nullptr};

    void createUI();
    void bindEvents();
};
