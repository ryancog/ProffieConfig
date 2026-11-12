#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/find.hpp
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

#include <wx/listbase.h>

#include "ui/dialog.hpp"
#include "ui/types.hpp"

struct EditorWindow;

struct FindDlg : pcui::Dialog {
    FindDlg(EditorWindow *);
    ~FindDlg() override;

private:
    enum {
        eID_Input = wxID_HIGHEST,
        eID_List,
    };

    pcui::DescriptorPtr ui();

    void bind_events();

    void on_entry(wxListEvent&);
    void on_find();

    struct Found {
        virtual ~Found();

        [[nodiscard]] virtual const std::vector<std::string>& path() const = 0;
        [[nodiscard]] virtual const std::string& snippet() const = 0;

        virtual void reveal() const = 0;
    };
    struct FoundStyle;

    data::prim::String m_find_text;
    data::prim::Integer m_num_found;
    std::vector<std::unique_ptr<Found>> m_found;
};

