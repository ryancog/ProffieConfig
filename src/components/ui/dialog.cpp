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
    const LabelData& title,
    long style
) {
    priv::tlw::bindOnCreate(this);
    priv::tlw::preCreate(this);

    Create(
        parent,
        id,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        style,
        "pcui::Dialog"
    );

    priv::tlw::postCreate(this);

    mTitleRcvr = priv::tlw::setTitle(this, title);

    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt) {
        if (evt.GetKeyCode() == WXK_ESCAPE) EndModal(wxID_CANCEL);
        // Don't always skip, otherwise, on macOS, the bell is rung.
        else evt.Skip();
    });
}

Dialog::~Dialog() {
    pcui::cripple(this);
}

void Dialog::Fit() {
    priv::tlw::fit<wxDialog>(this);
}

