#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/priv/groupbox.hpp
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

namespace pcui::priv {

/**
 * It's like a wxStaticBoxSizer, but cooler.
 *
 * Ensures cross-platform visual consistency.
 * macOS has a nice border around the box, but other platforms do not.
 */
class GroupBox : public wxStaticBox {
public:
    GroupBox();
    GroupBox(wxOrientation orient, wxWindow *parent, const wxString& label);

    void create(wxOrientation orient, wxWindow *parent, const wxString& label);

    wxSizer *sizer();
    wxWindow *childParent();

    bool Layout() final;

    wxSize DoGetBestClientSize() const final;

private:
    wxPanel *mPanel;
    wxSizer *mSizer;
};

} // namespace pcui::priv

