#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/dialog.h>

#include "ui/controls/filepicker.h"
#include "ui/controls/text.h"

#include "../../mainmenu/mainmenu.h"

class AddConfig : public wxDialog {
public:
    AddConfig(MainMenu *);
    enum {
        ID_CreateNew,
        ID_ImportExisting,
    };

    MainMenu *parent{nullptr};

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
