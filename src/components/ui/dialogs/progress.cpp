#include "progress.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/dialogs/progress.cpp
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

#include <wx/thread.h>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/dialog.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/helpers/if.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"

using namespace pcui;

ProgressDialog::ProgressDialog(
    wxWindow *parent,
    const wxString& title,
    bool mayCancel,
    wxSize size,
    long style
) : Dialog(parent, wxID_ANY, title, style) {
    mMessage.change(_("Initializing...").ToStdString());

    size.IncTo({200, 50});
    build(this, ui(mayCancel, size));

    if (parent) ShowWindowModal();
    else Show();
}

ProgressDialog::~ProgressDialog() {
    cripple(this);
}

void ProgressDialog::set(uint32 val, const wxString& message) {
    if (not message.empty()) {
        mMessage.change(message.ToStdString());
    }

    mData.set(val);
}

void ProgressDialog::range(uint32 val) {
    mData.range(val);
}

void ProgressDialog::pulse(const wxString& message) {
    if (not message.empty()) {
        mMessage.change(message.ToStdString());
    }

    mData.pulse();
}

void ProgressDialog::finish(bool modalWait, const wxString& message) {
    mMessage.change(message.ToStdString());

    { Progress::Data::Context ctxt{mData};
        ctxt.set(ctxt.range());
    }

    const auto doFinish{[this, modalWait] {
        // Make sure that layout updates have completed before locking things
        // up in the modal context.
        wxYield();

        // The data has been set as finished and the cancel button therefore is
        // hidden. Now check if the cancel button wasn't handled by the data
        // processor.
        if (data::context(mCancelled).val()) {
            // If it's been pressed, we don't wait in any case.
            Close(true);
            return;
        }

        if (modalWait) ShowModal();
        Close(true);
    }};

    if (wxIsMainThread()) {
        doFinish();
        return;
    }

    std::promise<void> promise;
    
    CallAfter([doFinish, &promise] {
        doFinish();
        promise.set_value();
    });

    promise.get_future().wait();
}

bool ProgressDialog::cancelled() {
    return data::context(mCancelled).val();
}

void ProgressDialog::show(bool show) {
    CallAfter([this, show] { Show(show); });
}

DescriptorPtr ProgressDialog::ui(bool mayCancel, wxSize size) {
    return Stack{
      .base_={
        .minSize_=size,
        .border_={.size_=winEdgeSpacing(), .dirs_=wxALL}
      },
      .children_={
#       ifdef __WXOSX__
        // On macOS, if we have a parent and are going to be shown WindowModal,
        // then the title won't be used (because this'll be displayed as a
        // sheet), so add the title to the content/UI directly.
        If{
          .cond_=GetParent() != nullptr,
          .then_={
            Label{
              .label_=GetTitle(),
              .font_=detail::FontData{Font::Normal}.makeFont().MakeBold(),
            }(),
            Spacer{.size_=interControlSpacing()}(),
          },
        }(),
#       endif
        Label{
          .label_=mMessage,
        }(),
        Spacer{.size_=4}(),
        Progress{
          .win_={.base_={.expand_=true}},
          .data_=mData,
          .showOnBar_=true,
        }(),
        Spacer{.size_=interControlSpacing()}(),
        DialogButtons{
          .ok_=Button{
            .win_={
              .show_=mData | Progress::Logic::Is_Done,
            },
            .label_=_("OK"),
            .func_=[this] {
                EndModal(wxID_OK);
            },
          }(),
          .cancel_=mayCancel ? Button{
            .win_={
              .show_=not (mData | Progress::Logic::Is_Done),
              .enable_=not (mCancelled | data::logic::IsSet{}),
            },
            .label_=_("Cancel"),
            .func_=[this] {
                mCancelled.set(true);
            }
          }() : nullptr,
        }(),
      }
    }();
}

