#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/message.h
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

#include <wx/msgdlg.h>
#include <wx/progdlg.h>

#include <utils/types.h>

#include "private/export.h"

namespace PCUI {

/**
 * Show a wxMessageBox (with platform-specific setup)
 *
 * Blocks until user has acknowledged the window.
 *
 * @param msg The main message within the box to show
 * @param caption The box window title
 * @param style Augmenting style flags
 * @param parent Window to be modal over
 *
 * @return user selection
 */
UI_EXPORT int32 showMessage(
    const wxString& msg,
    const wxString& caption = {},
    int32 style = wxOK | wxCENTER,
    wxWindow *parent = nullptr
);

struct HideableInfo {
    bool wantsToHide;
    int32 result;
};

UI_EXPORT HideableInfo showHideablePrompt(
    const wxString& msg,
    const wxString& caption,
    wxWindow *parent = nullptr,
    int32 style = wxOK | wxCENTER,
    const wxString& yesText = wxEmptyString,
    const wxString& noText = wxEmptyString,
    const wxString& okText = wxEmptyString,
    const wxString& cancelText = wxEmptyString
);

#ifdef __WXMSW__
using ProgressDialog = wxGenericProgressDialog;
#else
using ProgressDialog = wxProgressDialog;
#endif

} // namespace PCUI

