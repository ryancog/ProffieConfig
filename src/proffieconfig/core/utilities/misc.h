#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/core/utilities/misc.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/event.h>

#include "utils/types.h"

namespace misc {

class MessageBoxEvent : public wxCommandEvent {
public:
    MessageBoxEvent(
        wxEventType type,
        int32 id,
        wxString message,
        wxString caption,
        long style = wxOK | wxCENTER
    ) : wxCommandEvent(type, id),
        caption_{std::move(caption)},
        message_{std::move(message)},
        style_{style} {}

    wxString caption_;
    wxString message_;
    long style_;
};

extern const wxEventTypeTag<MessageBoxEvent> EVT_MSGBOX;

} // namespace misc

