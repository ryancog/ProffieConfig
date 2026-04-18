#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/ui/dialogs/message.hpp
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

#include "ui/dialogs/message_types.hpp"
#include "utils/types.hpp"

#include "ui_export.h"

namespace pcui {

/**
 * Show a wxMessageBox (with platform-specific setup)
 *
 * Blocks until user has acknowledged the window.
 *
 * @return user selection
 */
UI_EXPORT int32 showMessage(
    const wxString& msg, const dialogs::message::Args& = {}
);

struct HideableResult {
    bool wantsHide_;
    int32 id_;
};

UI_EXPORT HideableResult showHideablePrompt(
    const wxString& msg, const dialogs::message::Args& = {}
);

} // namespace pcui

