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

#include <future>

#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>

#include "app/app.hpp"
#include "ui/utils.hpp"

#if __WXMSW__
#include <wx/generic/richmsgdlgg.h>

// for dark mode
using MessageDialogType = wxGenericMessageDialog;
using RichMessageDialogType = wxGenericRichMessageDialog;
#else
using MessageDialogType = wxMessageDialog;
using RichMessageDialogType = wxRichMessageDialog;
#endif

int32 pcui::showMessage(
    const wxString& msg,
    const dialogs::message::Args& args
) {
    std::promise<int32> promise;

    const auto show_dialog{[&] {
        const auto& caption{args.caption_.empty()
            ? app::getName()
            : args.caption_
        };
        MessageDialogType dlg(
            args.parent_,
            msg,
            caption,
            static_cast<long>(args.style_)
        );

        dlg.SetOKCancelLabels(args.labels_.ok_, args.labels_.cancel_);
        dlg.SetYesNoLabels(args.labels_.yes_, args.labels_.no_);

        auto res{dlg.ShowModal()};
        promise.set_value([res] {
            switch (res) {
                case wxID_OK: return wxOK;
                case wxID_CANCEL: return wxCANCEL;
                case wxID_YES: return wxYES;
                case wxID_NO: return wxNO;
                case wxID_HELP: return wxHELP;
                default: return 0;
            }
        }());
    }};

    pcui::safe_call(show_dialog);
    return promise.get_future().get();
}

pcui::HideableResult pcui::showHideablePrompt(
    const wxString& msg,
    const dialogs::message::Args& args
) {
    std::promise<HideableResult> promise;

    const auto show_dialog{[&] {
        RichMessageDialogType dlg(
            args.parent_,
            msg,
            args.caption_.empty()
                ? app::getName()
                : args.caption_,
            static_cast<long>(args.style_)
        );

        dlg.SetOKCancelLabels(args.labels_.ok_, args.labels_.cancel_);
        dlg.SetYesNoLabels(args.labels_.yes_, args.labels_.no_);
        dlg.ShowCheckBox(_("Do Not Show Again"));

        HideableResult result;
        result.id_ = dlg.ShowModal();
        result.wantsHide_ = dlg.IsCheckBoxChecked();
        promise.set_value(result);
    }};

    pcui::safe_call(show_dialog);
    return promise.get_future().get();
}

