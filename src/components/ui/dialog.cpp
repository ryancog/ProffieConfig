#include "dialog.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/dialog.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/window.h>

#include "ui/build.hpp"
#include "ui/priv/tlw.hpp"

using namespace pcui;

Dialog::Dialog(
    wxWindow *parent,
    wxWindowID id,
    const wxString& title,
    long style
) {
    priv::tlw::bindOnCreate(this);
    priv::tlw::preCreate(this);

    Create(
        parent,
        id,
        title,
        wxDefaultPosition,
        wxDefaultSize,
        style,
        "pcui::Dialog"
    );

    priv::tlw::postCreate(this);

    // Normally the wxDialog EVT_CHAR_HOOK handler only will dismiss the dialog
    // via ESC if there's a button with GetEscapeId() id, and if there isn't,
    // then it'll try a couple other things, but in any case only if there's
    // associated buttons.
    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt) {
        if (evt.GetKeyCode() == WXK_ESCAPE) {
            wxCommandEvent event(wxEVT_BUTTON, GetEscapeId());
            ProcessEvent(event);
        } else  {
            // Don't always skip.
            // On macOS, the bell would be rung, and we don't want duplicate
            // processing anyways, that'd be silly, the event was handled.
            evt.Skip();
        }
    });
}

Dialog::~Dialog() {
    pcui::cripple(this);
}

void Dialog::Fit() {
    priv::tlw::fit<wxDialog>(this);
}

void Dialog::destroy_on_close() {
    Bind(wxEVT_CLOSE_WINDOW, &Dialog::on_close, this);
}

void Dialog::emulate_button(int id) {
    wxCommandEvent evt(wxEVT_BUTTON, id);
    evt.SetEventObject(this);
    ProcessEvent(evt);
}

void Dialog::on_close(wxCloseEvent&) {
    // Don't skip, the default handling should not be called.

    // This queues destruction, it is not immediate, which is what is wanted.
    Destroy();
}

void Dialog::on_button(wxCommandEvent& evt) {
    // If it was either of the standard "finish" buttons, close.
    if (
            evt.GetId() == GetAffirmativeId() or
            evt.GetId() == GetEscapeId() or
            // This isn't the most intuitive logic to me, but that's what
            // wxDialogBase::OnButton() checks, and this function should be an
            // overlay over that, since it'll forward to it if not vetoed.
            (evt.GetId() == wxID_CANCEL and GetEscapeId() == wxID_ANY)
       ) {
        if (Close())
            // If the close was not vetoed, then pass along the event for usual
            // handling (ending modality, etc.)
            evt.Skip();
    } else {
        // Some other non-"finish" button, wxDialogBase::OnButton() should be
        // called.
        evt.Skip();
    }
}

