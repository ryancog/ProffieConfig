#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/dialogs/progress.hpp
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

#include "data/primitive/models/bool.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/dialog.hpp"
#include "ui/indication/progress.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT ProgressDialog : private Dialog {
    ProgressDialog(
        wxWindow *parent,
        const wxString& title, 
        bool mayCancel = false,
        wxSize size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE
    );

    ~ProgressDialog() override;

    void set(uint32, const wxString& = {});
    void range(uint32);

    void pulse(const wxString& = {});

    void finish(bool modalWait = true, const wxString& = _("Done"));

    bool cancelled();

    void show(bool = true);
    void hide() { show(false); }

private:
    DescriptorPtr ui(bool, wxSize);

    data::prim::Bool mCancelled;
    data::prim::String mMessage;
    Progress::Data mData;
};

} // namespace pcui

