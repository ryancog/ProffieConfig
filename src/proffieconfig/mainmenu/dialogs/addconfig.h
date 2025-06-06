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

        ID_ChooseConfig,
        ID_ConfigName,
    };

    MainMenu* parent{nullptr};
    wxString existingPath;
    wxString configName;

    wxToggleButton* createNew{nullptr};
    wxToggleButton* importExisting{nullptr};
    wxFilePickerCtrl* chooseConfig{nullptr};
    PCUI::Text* configNameEntry{nullptr};
    void update();

private:
    wxStaticText* mChooseConfigText{nullptr};

    wxStaticText* mInvalidNameWarning{nullptr};
    wxStaticText* mDuplicateWarning{nullptr};
    wxStaticText* mFileSelectionWarning{nullptr};

    void createUI();
    void bindEvents();
};
