#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../dialogs/awarenessdlg.h"

class BladesPage : public wxStaticBoxSizer, PCUI::Notifier {
public:
    BladesPage(EditorWindow *);

private:
    EditorWindow *mParent{nullptr};

    enum {
        ID_OpenBladeAwareness,

        ID_AddArray,
        ID_RemoveArray,
        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSubBlade,
        ID_RemoveSubBlade,

        ID_Star1Box,
        ID_Star2Box,
        ID_Star3Box,
        ID_Star4Box,
        ID_NoSelectText,
        ID_PinNameAdd,
    };

    BladeAwarenessDlg *mAwarenessDlg{nullptr};

    void bindEvents();
    void handleNotification(uint32) final;

    wxSizer *createBladeSelect();
    wxSizer *createBladeSettings();
};
