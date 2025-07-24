#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../dialogs/customoptionsdlg.h"

class GeneralPage : public wxStaticBoxSizer, PCUI::Notifier {
public:
    GeneralPage(EditorWindow*);

    enum {
        ID_CustomOptions = 2,
        ID_OSVersion,
    };

private:
    EditorWindow* mParent{nullptr};
    CustomOptionsDlg *mCustomOptDlg{nullptr};

    void bindEvents();
    void handleNotification(uint32) final;

    wxStaticBoxSizer* setupSection(wxStaticBoxSizer*);
    wxStaticBoxSizer* optionSection(wxStaticBoxSizer*);
    wxBoxSizer* rightOptions(wxStaticBoxSizer*);
    wxBoxSizer* leftOptions(wxStaticBoxSizer*);
};
