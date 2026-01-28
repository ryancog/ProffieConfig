#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/dialog.h>
#include <wx/panel.h>

#include "ui/notifier.h"

#include "../editorwindow.h"

class CustomOptionsDlg : public wxDialog, pcui::NotifyReceiver {
public:
    CustomOptionsDlg(EditorWindow*);

private:
    void handleNotification(uint32) final;

    EditorWindow *mParent;

    enum {
        ID_AddDefine,
    };

    wxScrolledWindow *mOptionArea{nullptr};
    wxButton *mAddDefineButton{nullptr};

    class CDefine;

    void bindEvents();
    void createUI();
    void createOptionArea();

    wxBoxSizer *header();
    static wxWindow *info(wxWindow*);
};

class CustomOptionsDlg::CDefine : public wxPanel {
public:
    CDefine(
        wxScrolledWindow *,
        Config::Config&,
        Config::Settings::CustomOption&
    );
};
