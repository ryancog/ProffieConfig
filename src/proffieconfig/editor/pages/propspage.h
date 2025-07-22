#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"
#include "ui/notifier.h"

#include <wx/scrolwin.h>
#include <wx/statbox.h>

// Forward declaration to get around circular dependency
class PropFile;

class PropsPage : public wxStaticBoxSizer, PCUI::Notifier {
public:
    PropsPage(EditorWindow *);

    enum {
        ID_Buttons,
        ID_PropInfo,

        ID_PropSelection,
    };

private:
    wxScrolledWindow* mPropsWindow{nullptr};

    EditorWindow* mParent{nullptr};

    vector<PropFile*> mProps;

    void loadProps();
    void bindEvents();

    void handleNotification(uint32) final;
    PCUI::NotifierData mNotifyData;
};
