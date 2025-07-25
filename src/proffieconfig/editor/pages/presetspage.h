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

class PresetsPage : public wxPanel, PCUI::Notifier {
public:
    PresetsPage(EditorWindow *);

    enum {
        ID_AddPreset,
        ID_RemovePreset,
        ID_MovePresetUp,
        ID_MovePresetDown,

        ID_AddArray,
        ID_RemoveArray,
    };

private:
    EditorWindow *mParent{nullptr};

    wxBoxSizer *mInjectionsSizer;


    void createUI();
    void bindEvents();
    void handleNotification(uint32 id);

    void rebuildInjections();
};
