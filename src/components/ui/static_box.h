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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "utils/types.h"

#include "ui_export.h"

namespace pcui {

/**
 * It's like a wxStaticBoxSizer, but cooler.
 *
 * Ensures cross-platform visual consistency.
 * macOS has a nice border around the box, but other platforms do not.
 */
class UI_EXPORT StaticBox : public wxStaticBox {
public:
    StaticBox(wxOrientation, wxWindow *, const wxString& = wxEmptyString);

    wxSizer *sizer() { return mSizer; }
    wxWindow *childParent() { return mPanel; }

    bool Layout() final;
    [[nodiscard]] wxSize DoGetBestClientSize() const final;

    // NOLINTBEGIN(readability-identifier-naming)
    wxSizerItem *Add(wxWindow *, const wxSizerFlags& = {});
    wxSizerItem *Add(wxSizer *, const wxSizerFlags& = {});
    wxSizerItem *AddSpacer(int32 size);
    wxSizerItem *AddStretchSpacer(int32 prop = 1);

    void Clear(bool deleteWindows = false);
    bool IsEmpty();
    // NOLINTEND(readability-identifier-naming)
    
private:
    wxPanel *mPanel;
    wxSizer *mSizer;
};

} // namespace pcui

