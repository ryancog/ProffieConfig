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
#include <wx/evtloop.h>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/dialog.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"

#if __WXOSX__
#include "ui/helpers/if.hpp"
#endif

using namespace pcui;

// NOTE: This uses a Dialog (and not a Frame) because the ShowWindowModal()
// behavior is used. Since it's always supposed to be heap-allocated though, the
// close and button handling needs to be overridden in order to close/destroy.
//
// I'm not a huge fan of the fact that there's not a straightforward way to
// handle a heap-created dialog.
//
// Previously, I tried to just hook EVT_SHOW to catch hiding and destroy then,
// but that ignores the fact that the dialog may be legitimately hidden other
// times besides close, so the more cumbersome approach of handling all the
// "close" flows is needed.

ProgressDialog::ProgressDialog(
    wxWindow *parent,
    const wxString& title,
    bool mayCancel,
    wxSize size,
    long style
) : Dialog(parent, wxID_ANY, title, style) {
    mMessage.change(_("Initializing...").utf8_string());

    size.IncTo({200, 50});
    build(this, ui(mayCancel, size));

    // Intercept the usual Dialog handling (which calls EndDialog)
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent&) {
        Destroy();
    });

    // Override the char hook handling to close instead of passing along to the
    // dialog functions which will just hide.
    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt) {
        if (evt.GetKeyCode() == WXK_ESCAPE)
            Close();
        else 
            // Don't always skip, otherwise, on macOS, the bell is rung.
            evt.Skip();
    });

    if (parent) {
#   if __WXOSX__
        // macOS has special sheet handling which allows this call to return and
        // processing to continue. Since other platforms would block, use normal
        // Show().
        ShowWindowModal();
#   else
        // Mimic window modality by disabling parent.
        parent->Disable();
        Bind(wxEVT_DESTROY, [parent](wxWindowDestroyEvent& evt) {
            evt.Skip();

            // NOTE: Reparenting shouldn't be an issue, but if it ever is, this
            // will break!
            parent->Enable();
        });

        Show();
#   endif
    } else {
        Show();
    }
}

ProgressDialog::~ProgressDialog() {
    cripple(this);
}

void ProgressDialog::set(uint32 val, const wxString& message) {
    if (not message.empty()) {
        mMessage.change(message.utf8_string());
    }

    mData.set(val);
}

void ProgressDialog::range(uint32 val) {
    mData.range(val);
}

void ProgressDialog::pulse(const wxString& message) {
    if (not message.empty()) {
        mMessage.change(message.utf8_string());
    }

    mData.pulse();
}

void ProgressDialog::finish(bool modalWait, const wxString& message) {
    mMessage.change(message.utf8_string());

    // The value is set to the end of the range in order to make the "OK"
    // button appear. If not modal waiting, then there's no point to doing
    // this, and it probably will just show the button unnecessarily.
    if (modalWait) {
        auto ctxt{data::context(mData)};
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
            EndDialog(wxID_CANCEL);
            return;
        }

        if (not modalWait) {
            EndDialog(wxID_OK);
            return;
        }

        wxGUIEventLoop loop;

        Bind(wxEVT_DESTROY, [&loop](wxWindowDestroyEvent& evt) {
            evt.Skip();
            loop.Exit();
        });

        Show();
        loop.Run();
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

wxWindow *ProgressDialog::parent() const {
    return GetParent();
}

wxString ProgressDialog::title() const {
    return GetTitle();
}

bool ProgressDialog::cancellable() const {
    return FindWindow(wxID_CANCEL);
}

DescriptorPtr ProgressDialog::ui(bool mayCancel, wxSize size) {
    return Stack{
      .base_={
        .minSize_=size,
        .border_={.size_=winEdgeSpacing(), .dirs_=wxALL}
      },
      .children_={
#       if __WXOSX__
        // On macOS, if we have a parent and are going to be shown WindowModal,
        // then the title won't be used (because this'll be displayed as a
        // sheet), so add the title to the content/UI directly.
        If{
          .cond_=GetParent() != nullptr,
          .then_={
            Label{
              .label_=GetTitle(),
              .font_=(- Font::Normal).MakeBold(),
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
                // Do this manually, normal handling for a dialog won't.
                Close();
            }
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

