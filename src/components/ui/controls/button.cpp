#include "button.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/button.cpp
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

#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/window.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/helpers.hpp"
#include "ui/detail/datawin.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxButton, data::String::Receiver> {
    Control(const detail::Scaffold& scaffold, const Button& desc) :
        func_{desc.func_} {

        long style{0};
        if (desc.exactFit_) style |= wxBU_EXACTFIT;

        const data::Model *modelPtr{nullptr};

        switch (desc.style_) {
            using enum Button::Style;
            case Normal:
                style |= wxBORDER_DEFAULT;
                break;
            case Companion:
                // This only does something on macOS though.
                style |= wxBORDER_SIMPLE;
                break;
        }

        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            style 
        );

        postCreation(scaffold, desc.win_);

        if (const auto *ptr{std::get_if<1>(&desc.label_)}) {
            data::String::ROContext str{*ptr};
            SetLabel(str.val());
            modelPtr = &ptr->get();
        } else {
            SetLabel(std::get<0>(desc.label_));
        }

        if (desc.bitmap_.src_) {
            bmp_ = desc.bitmap_.src_;

            const auto updateBitmap{[this]() {
                SetBitmap(bmp_.realize());
            }};

            Bind(
                wxEVT_SYS_COLOUR_CHANGED,
                [updateBitmap](wxSysColourChangedEvent& evt) {
                    evt.Skip();
                    updateBitmap();
                }
            );

            updateBitmap();
        }

        if (desc.default_) SetDefault();

        if (modelPtr) attach(*modelPtr);
        Bind(wxEVT_BUTTON, &Control::onPress, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void SetLabel(const wxString& str) override {
#       ifdef __WXGTK__
        // TODO: Does this look right?
        const auto exactFit{GetWindowStyle() & wxBU_EXACTFIT};
        wxButton::SetLabel(exactFit and not str.empty()
            ? ' ' + str + ' '
            : str
        );
#       else
        wxButton::SetLabel(str);
#       endif

        layoutAndFitFor(this);
    }

    void onPress(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        callback();
    }

    void onChange() override {
        safeCall([this, str=context<data::String>().val()]() {
            SetLabel(str);
        });
    }

    void callback() {
        if (const auto *ptr{std::get_if<0>(&func_)}) {
            if (*ptr) (*ptr)();
        } else if (const auto *ptr{std::get_if<1>(&func_)}) {
            if (*ptr) {
                CallbackContext ctxt{
                    .window_ = this,
                    .topLevel_ = static_cast<wxTopLevelWindow *>(
                        wxGetTopLevelParent(this)
                    ),
                };

                (*ptr)(ctxt);
            }
        }
    }

    Bitmap bmp_;
    const Button::MaybeContextualCallback func_;
};

} // namespace

std::unique_ptr<detail::Descriptor> Button::operator()() {
    // The proper in-button size for a bitmap depends on the platform
    if (bitmap_.mode_ == BitmapMode::Clamped and bitmap_.src_) {
#       if defined(__WXOSX__)
        bitmap_.src_.pad(1, 16, wxVERTICAL);
#       elif defined(__WXGTK__)
        bitmap_.src_.pad(2, 16, wxVERTICAL);
#       elif defined(__WXMSW__)
        bitmap_.src_.scaleTo(12, wxVERTICAL);
#       endif
    }

    return std::make_unique<Button::Desc>(std::move(*this));
}

Button::Desc::Desc(Button&& data) :
    Button{std::move(data)} {}

wxSizerItem *Button::Desc::build(const detail::Scaffold& scaffold) const {
    auto *button{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(button)};
    detail::apply(win_.base_, item);
    return item;
}

