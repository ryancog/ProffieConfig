#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include "../editorwindow.h"

#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

// Forward declaration to get around circular dependency
class PropFile;

class PropsPage : public wxStaticBoxSizer {
public:
    PropsPage(EditorWindow *);

    enum {
        ID_Buttons,
        ID_PropInfo,
    };

private:
    wxScrolledWindow* mPropsWindow{nullptr};

    EditorWindow* mParent{nullptr};

    vector<PropFile*> mProps;

    void loadProps();
    void bindEvents();
};
