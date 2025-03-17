#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/combobox.h>

#include "../editor/editorwindow.h"
#include "../mainmenu/mainmenu.h"
#include "log/branch.h"

namespace Arduino {
    void refreshBoards(MainMenu*);
    void applyToBoard(MainMenu*, EditorWindow*);
    void verifyConfig(wxWindow*, EditorWindow*);

    void init(wxWindow *);
    vector<wxString> getBoards(Log::Branch&);

    enum Proffieboard {
        PROFFIEBOARDV3 = 0,
        PROFFIEBOARDV2,
        PROFFIEBOARDV1,
    };

    struct Event : wxEvent {
        Event(wxEventType type) : wxEvent(wxID_ANY, type) {}

        [[nodiscard]] wxEvent *Clone() const override { return new Event(*this); }

        bool succeeded{false};
        string str;
    };

    wxDECLARE_EVENT(EVT_INIT_DONE, Event);
    wxDECLARE_EVENT(EVT_APPLY_DONE, Event);
    wxDECLARE_EVENT(EVT_VERIFY_DONE, Event);
    wxDECLARE_EVENT(EVT_REFRESH_DONE, Event);
    wxDECLARE_EVENT(EVT_CLEAR_BLIST, Event);
    wxDECLARE_EVENT(EVT_APPEND_BLIST, Event);
} // namespace Arduino
