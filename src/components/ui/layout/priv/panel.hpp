#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/priv/panel.hpp
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

namespace pcui::priv {

class Panel : public wxPanel {
public:
    Panel() = default;
    Panel(
        wxWindow *parent,
        long style = wxTAB_TRAVERSAL,
        const wxString& name = wxPanelNameStr
    );

    bool Create(
        wxWindow *,
        wxWindowID,
        const wxPoint&,
        const wxSize&,
        long,
        const wxString&
    ) = delete;

    void create(
        wxWindow *parent,
        long style = wxTAB_TRAVERSAL,
        const wxString& name = wxPanelNameStr
    );
};

} // namespace pcui::priv

