#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

#include "../editorwindow.h"

class PresetsPage : public wxStaticBoxSizer {
public:
    PresetsPage(EditorWindow *);

    enum {
        ID_AddPreset,
        ID_RemovePreset,
        ID_MovePresetUp,
        ID_MovePresetDown
    };

private:
    EditorWindow *mParent{nullptr};

    wxBoxSizer *mInjectionsSizer;

    void bindEvents();

    wxBoxSizer *createPresetSelect();
    wxBoxSizer *createPresetConfig();

    void pushIfNewPreset();
    void rebuildBladeArrayList();
    void rebuildPresetList();
    void rebuildBladeList();
    void resizeAndFillPresets();
    void updateFields();

    void stripAndSaveEditor();
    void stripAndSaveComments();
    void stripAndSaveName();
    void stripAndSaveDir();
    void stripAndSaveTrack();

    void rebuildInjections();
};
