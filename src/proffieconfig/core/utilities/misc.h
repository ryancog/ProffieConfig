#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

namespace Misc {
    class MessageBoxEvent;

    extern const wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;
} // namespace Misc

class Misc::MessageBoxEvent : public wxCommandEvent {
public:
    MessageBoxEvent(int32_t id, wxString _message, wxString _caption, long _style = wxOK | wxCENTER){
        this->SetEventType(EVT_MSGBOX);
        this->SetId(id);
        caption = std::move(_caption);
        message = std::move(_message);
        style = _style;
    }

    wxString caption;
    wxString message;
    long style;
};
