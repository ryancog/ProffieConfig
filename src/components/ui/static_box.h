#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/static_box.h
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

#include <wx/sizer.h>
#include <wx/statbox.h>

#include "utils/types.h"

#include "ui_export.h"

namespace PCUI {

/**
 * Simple wrapper class to make sure there's consistent spacing inside a wxStaticBox
 */
class UI_EXPORT StaticBox : public wxStaticBoxSizer {
public:
    StaticBox(wxOrientation, wxWindow *, const wxString& = wxEmptyString);

    // NOLINTBEGIN(readability-identifier-naming)
    virtual void SetFocus() { GetStaticBox()->SetFocus(); }
    virtual void SetMinSize(const wxSize& size) { GetStaticBox()->SetMinSize(size); }

    template <typename EventTag, typename Functor>
    void Bind(
        const EventTag& eventType,
        const Functor &functor,
        int winid = wxID_ANY,
        int lastId = wxID_ANY,
        wxObject *userData = nullptr
    ) {
        GetStaticBox()->Bind(
            eventType,
            functor,
            winid,
            lastId,
            userData
        );
    }

    template <typename EventTag, typename EventArg>
    void Bind(
        const EventTag& eventType,
        void (*function)(EventArg &),
        int winid = wxID_ANY,
        int lastId = wxID_ANY,
        wxObject *userData = nullptr
    ) {
        GetStaticBox()->Bind(
            eventType,
            function,
            winid,
            lastId,
            userData
        );
    }
    // NOLINTEND(readability-identifier-naming)

#   ifndef __WXOSX__
    wxSizerItem *Add(wxWindow *, const wxSizerFlags& = {});
    wxSizerItem *Add(wxSizer *, const wxSizerFlags& = {});
    wxSizerItem *AddSpacer(int32 size) override;
    wxSizerItem *AddStretchSpacer(int32 prop);
    void Clear(bool deleteWindows) override;
    bool IsEmpty();
#   endif

#   ifndef __WXOSX__
private:
    wxBoxSizer *mSizer;
#   endif
};

} // namespace PCUI

