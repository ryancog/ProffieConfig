#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024-2025 Ryan Ogurek

#include <map>

#include <wx/button.h>
#include <wx/combobox.h>

#include "ui/frame.h"
#include "ui/notifier.h"

#include "../editor/editorwindow.h"

namespace Onboard {

class Overview;

} // namespace Onboard

class MainMenu : public PCUI::Frame, private PCUI::Notifier {
public:
    static MainMenu* instance;
    MainMenu(wxWindow * = nullptr);

    void removeEditor(EditorWindow *);

    PCUI::ChoiceData boardSelection;
    PCUI::ChoiceData configSelection;

    enum {
        // on macOS menu items cannot have ID 0
        // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl?
        ID_Copyright = 2,
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

        ID_BoardSelection,
        ID_ConfigSelection,
    };

private:
    PCUI::NotifierData mNotifyData;
    vector<EditorWindow *> mEditors;

    friend Onboard::Overview;
    void createUI();
    void createMenuBar();
    void bindEvents();

    void handleNotification(uint32 id) final;
};
