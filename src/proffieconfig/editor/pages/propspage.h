#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

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
    PropsPage(wxWindow*);

    void update();
    static void updateDisables(PropFile*);
    void updateProps();
    void updateSelectedProp(const string& = "");
    PropFile* getSelectedProp();
    const vector<PropFile*>& getLoadedProps();
    wxScrolledWindow* propsWindow{nullptr};

    PCUI::Choice* propSelection{nullptr};
    wxButton* buttonInfo{nullptr};
    wxButton* propInfo{nullptr};

    enum {
        ID_PropSelect,
        ID_Option,
        ID_Buttons,
        ID_PropInfo,
    };
private:
    EditorWindow* mParent{nullptr};
    wxBoxSizer *mTopSizer{nullptr};

    vector<PropFile*> mProps;

    void loadProps();
    void bindEvents();
};
