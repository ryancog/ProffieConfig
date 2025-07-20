#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <map>

#include <wx/button.h>
#include <wx/combobox.h>

#include "ui/frame.h"

#include "../editor/editorwindow.h"

namespace Onboard {

class Overview;

} // namespace Onboard

class MainMenu : public PCUI::Frame {
public:
    static MainMenu* instance;
    MainMenu(wxWindow * = nullptr);

    void removeEditor(EditorWindow *);

    wxButton* applyButton{nullptr};
    wxButton* openSerial{nullptr};
    wxButton* removeConfig{nullptr};
    wxButton* editConfig{nullptr};

    PCUI::ChoiceData boardSelection;
    PCUI::ChoiceData configSelection;

    std::map<std::shared_ptr<Config::Config>, EditorWindow*> editors;

    enum {
        ID_DUMMY1, // on macOS menu items cannot have ID 0
        ID_DUMMY2, // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl? This is a workaround.

        ID_Copyright,
        ID_ReRunSetup,

        ID_Docs,
        ID_Issue,
        ID_Logs,

        ID_UpdateManifest,
        ID_AddProp,

        ID_RefreshDev,
        ID_ApplyChanges,

        ID_OpenSerial,

        ID_AddConfig,
        ID_RemoveConfig,
        ID_EditConfig,
    };

private:

    friend Onboard::Overview;
    void createUI();
    void createMenuBar();
    void bindEvents();

    EditorWindow *generateEditor(const string& configName);
};
