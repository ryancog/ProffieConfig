#include "message.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/ui/dialogs/message.cpp
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
#include <wx/richmsgdlg.h>
#ifdef __WXMSW__
#include <wx/generic/richmsgdlgg.h>
#endif

#include "app/app.hpp"

int32 pcui::showMessage(
    const wxString& msg,
    const dialogs::message::Args& args
) {
    const auto& caption{args.caption_.empty() ? app::getName() : args.caption_};

#   ifdef __WXMSW__
    // for dark mode
    wxGenericMessageDialog dlg(args.parent_, msg, caption, args.style_);
#   else
    wxMessageDialog dlg(args.parent_, msg, caption, args.style_);
#   endif

    dlg.SetOKCancelLabels(args.labels_.ok_, args.labels_.cancel_);
    dlg.SetYesNoLabels(args.labels_.yes_, args.labels_.no_);

    auto ret{dlg.ShowModal()};
    switch (ret) {
        case wxID_OK: return wxOK;
        case wxID_CANCEL: return wxCANCEL;
        case wxID_YES: return wxYES;
        case wxID_NO: return wxNO;
        case wxID_HELP: return wxHELP;
        default: return 0;
    }
}

pcui::HideableResult pcui::showHideablePrompt(
    const wxString& msg,
    const dialogs::message::Args& args
) {
    const auto& caption{args.caption_.empty() ? app::getName() : args.caption_};

#   ifdef __WXMSW__
    wxGenericRichMessageDialog dlg(args.parent_, msg, caption, args.style_);
    );
#   else
    wxRichMessageDialog dlg(args.parent_, msg, caption, args.style_);
#   endif

    dlg.SetOKCancelLabels(args.labels_.ok_, args.labels_.cancel_);
    dlg.SetYesNoLabels(args.labels_.yes_, args.labels_.no_);

    dlg.ShowCheckBox(_("Do Not Show Again"));

    HideableResult ret;
    ret.id_ = dlg.ShowModal();
    ret.wantsHide_ = dlg.IsCheckBoxChecked();
    return ret;
}

