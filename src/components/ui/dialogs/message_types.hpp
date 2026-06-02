#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/dialogs/message_types.hpp
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

#include "utils/types.hpp"

/**
 * This is broken out into another file w/o requiring the export header solely
 * for the app component to include.
 */
namespace pcui::dialogs::message {

struct Labels {
    wxMessageDialog::ButtonLabel yes_{wxID_YES};
    wxMessageDialog::ButtonLabel no_{wxID_NO};
    wxMessageDialog::ButtonLabel ok_{wxID_OK};
    wxMessageDialog::ButtonLabel cancel_{wxID_CANCEL};
};

struct Args {
    /**
     * Window title
     */
    wxString caption_;
    /**
     * Augment how/what is shown in the box.
     */
    uint64 style_{wxOK | wxCENTER};
    /**
     * Alternative button labels
     */
    Labels labels_{};
    /**
     * Window to be modal over
     */
    wxWindow *parent_{nullptr};
};

} // namespace pcui::dialogs::message

