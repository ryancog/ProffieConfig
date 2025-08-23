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
                wxString message,
                wxString caption,
                int64 style = wxOK | wxCENTER
            ) : wxCommandEvent(type, id),
                caption{std::move(caption)},
                message{std::move(message)},
                style{style} {}

            wxString caption;
            wxString message;
            int64 style;
    };

    extern const wxEventTypeTag<MessageBoxEvent> EVT_MSGBOX;
} // namespace Misc

