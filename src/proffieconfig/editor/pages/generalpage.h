#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "ui/controls/numeric.h"
#include "ui/controls/toggle.h"

#include "../editorwindow.h"
#include "../dialogs/customoptionsdlg.h"

class GeneralPage : public wxStaticBoxSizer {
public:
    GeneralPage(EditorWindow*);

    enum {
        ID_CustomOptions = 2
    };

private:
    EditorWindow* mParent{nullptr};
    CustomOptionsDlg *mCustomOptDlg{nullptr};

    void bindEvents();
    void createToolTips() const;

    wxStaticBoxSizer* boardSection(wxStaticBoxSizer*);
    wxStaticBoxSizer* optionSection(wxStaticBoxSizer*);
    wxBoxSizer* rightOptions(wxStaticBoxSizer*);
    wxBoxSizer* leftOptions(wxStaticBoxSizer*);
};
