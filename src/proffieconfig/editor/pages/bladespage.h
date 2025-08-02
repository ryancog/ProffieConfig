#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../dialogs/awarenessdlg.h"

class ArrayEditDlg;

class BladesPage : public wxPanel, PCUI::Notifier {
public:
    BladesPage(EditorWindow *);

private:
    EditorWindow *mParent{nullptr};

    enum {
        ID_OpenBladeAwareness,

        ID_IssueIcon,

        ID_EditArray,
        ID_AddArray,
        ID_RemoveArray,

        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSplit,
        ID_RemoveSplit,

        ID_NoSelectText,
        ID_PinNameAdd,
    };

    BladeAwarenessDlg *mAwarenessDlg{nullptr};

    void bindEvents();
    void handleNotification(uint32) final;

    wxSizer *createBladeSelect();
    wxSizer *createBladeSettings();
   
    wxSizer *mSimpleSizer;
    wxSizer *mPixelSizer;
};
