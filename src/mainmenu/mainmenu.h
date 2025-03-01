// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"

#include <wx/frame.h>
#include <wx/button.h>
#include <wx/combobox.h>

namespace Onboard {
class Overview;
}

class MainMenu : public wxFrame {
public:
    static MainMenu* instance;
    MainMenu(wxWindow* = nullptr);

    void removeEditor(EditorWindow *);
    void update();

    wxButton* refreshButton{nullptr};
    wxButton* applyButton{nullptr};
    pcChoice* boardSelect{nullptr};

    wxButton* openSerial{nullptr};

    pcChoice* configSelect{nullptr};
    wxButton* addConfig{nullptr};
    wxButton* removeConfig{nullptr};
    wxButton* editConfig{nullptr};

    EditorWindow* activeEditor{nullptr};
    std::vector<EditorWindow*> editors{};

    enum {
        ID_DUMMY1, // on macOS menu items cannot have ID 0
        ID_DUMMY2, // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl? This is a workaround.
        ID_Copyright,
        ID_ReRunSetup,
        ID_RefreshDev,
        ID_ApplyChanges,
        ID_DeviceSelect,
        ID_Docs,
        ID_Issue,

        ID_OpenSerial,

        ID_ConfigSelect,
        ID_AddConfig,
        ID_RemoveConfig,
        ID_EditConfig,

        ID_AddProp,
    };

private:
    friend Onboard::Overview;
    void createUI();
    void createMenuBar();
    void createTooltips();
    void bindEvents();

    EditorWindow *generateEditor(const std::string& configName);
};
