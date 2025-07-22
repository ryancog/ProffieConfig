#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

#include "utils/types.h"

namespace Misc {
    class MessageBoxEvent : public wxCommandEvent {
        public:
            MessageBoxEvent(
                wxEventType type,
                int32 id,
                wxString _message,
                wxString _caption,
                long _style = wxOK | wxCENTER
            ) : wxCommandEvent(type, id) {
                caption = std::move(_caption);
                message = std::move(_message);
                style = _style;
            }

            wxString caption;
            wxString message;
            long style;
    };

    extern const wxEventTypeTag<MessageBoxEvent> EVT_MSGBOX;
} // namespace Misc

