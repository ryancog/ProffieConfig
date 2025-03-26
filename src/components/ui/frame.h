#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/frame.h
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

#include <wx/frame.h>
#include <wx/panel.h>

#include <app/app.h>

#include "private/export.h"

namespace PCUI {

class UI_EXPORT Frame : public wxFrame {
public:
    Frame(wxWindow* parent,
          int32_t winID,
          const string& title,
          const wxPoint& pos = wxDefaultPosition,
          const wxSize& size = wxDefaultSize,
          int32_t style = wxDEFAULT_FRAME_STYLE,
          const string& name = "Frame");
    ~Frame() override;

    [[nodiscard]] App::Menus getMenus();

    void setReference(Frame**);

private:
    App::Menus mMenus;
    Frame** mReference{nullptr};
};

} // namespace PCUI
