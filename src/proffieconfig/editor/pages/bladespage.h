#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../dialogs/awarenessdlg.h"

class BladesPage : public wxStaticBoxSizer {
public:
    BladesPage(EditorWindow *);

private:
    EditorWindow *mParent{nullptr};

    enum {
        ID_OpenBladeAwareness,
        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSubBlade,
        ID_RemoveSubBlade,
        ID_AddPowerPinSelection,
    };

    BladeAwarenessDlg *mAwarenessDlg{nullptr};

    void bindEvents();

    wxBoxSizer *createBladeSelect();
    wxBoxSizer *createBladeManager();
    wxBoxSizer *createBladeSetup();
    wxBoxSizer *createBladeSettings();
};
