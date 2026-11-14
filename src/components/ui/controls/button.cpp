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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/helpers.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxButton> {
    Control(const detail::Scaffold& scaffold, const Button& desc) :
        func_{desc.func_} {

        long style{0};
        if (desc.exactFit_)
            style |= wxBU_EXACTFIT;

        detail::addButtonStyle(desc.style_, style);

        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            style 
        );

        postCreation(scaffold, desc.win_);

        if (desc.bitmap_.src_) {
            bmp_ = desc.bitmap_.src_;

#           if __WXGTK__
            // Images are shown on the left on other platforms, so even though
            // it means aligning everything to the left on GTK, make it
            // consistent. IMO centered (and thus unaligned) images just look
            // bad.
            style |= wxBU_LEFT;
#           endif

            const auto update_bitmap{[this]() {
                SetBitmap(bmp_.realize());
#               if __WXMSW__
                // I don't really understand how these values correlate to the
                // underlying calculation, because it's not totally
                // straightforward, but this provides more reasonable padding
                // for Windows, particularly with larger bitmaps.
                SetBitmapMargins(6, 0);
#               endif
            }};

            Bind(
                wxEVT_SYS_COLOUR_CHANGED,
                [update_bitmap](wxSysColourChangedEvent& evt) {
                    evt.Skip();
                    update_bitmap();
                }
            );

            update_bitmap();
        }

        SetFont(desc.font_.makeFont());

        if (const auto *ptr{std::get_if<1>(&desc.label_)}) {
            str_ = &ptr->get();
        } else {
            SetLabel(std::get<0>(desc.label_));
        }

        if (desc.default_)
            SetDefault();

        if (str_) {
            static const auto table{[] {
                data::base::String::RecvTable table;
                table.onEnable_ = data::map<&DataWindow::onEnable>();
                table.onFocus_ = data::map<&DataWindow::onFocus>();
                table.onChange_ = data::map<&Control::on_change>();
                return table;
            }()};
            observeWith(*str_, table);
        }

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        if (str_)
            SetLabel(data::context(*str_).val());

        Bind(wxEVT_BUTTON, &Control::on_press, this);
    }

    const data::base::Model *primaryModel() override {
        return str_;
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

    void on_press(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en)
            return;

        if (const auto *ptr{std::get_if<0>(&func_)}) {
            if (*ptr)
                (*ptr)();
            else
                evt.Skip();
        } else if (const auto *ptr{std::get_if<1>(&func_)}) {
            if (*ptr) {
                CallbackContext ctxt{
                    .window_ = this,
                    .topLevel_ = static_cast<wxTopLevelWindow *>(
                        wxGetTopLevelParent(this)
                    ),
                };

                (*ptr)(ctxt);
            } else {
                evt.Skip();
            }
        }
    }

    void on_change() {
        safeCall([this, str=data::context(*str_).val()]() {
            SetLabel(str);
        });
    }

    Bitmap bmp_;
    const data::base::String *str_{nullptr};
    const Button::MaybeContextualCallback func_;
};

} // namespace

DescriptorPtr Button::operator()() {
    if (bitmap_.src_) {
        // The proper in-button size for a bitmap depends on the platform
        switch (bitmap_.mode_) {
            case BitmapMode::Clamped:
#               if __WXOSX__ or __WXGTK__
                bitmap_.src_.pad(1, 16, wxVERTICAL);
#               elif defined(__WXMSW__)
                bitmap_.src_.scaleTo(12, wxVERTICAL);
#               endif
                break;
            case BitmapMode::Maintain:
#               if __WXGTK__
                bitmap_.src_.pad(2);
#               endif

                if (win_.base_.minSize_.GetHeight() == wxDefaultCoord)
                    win_.base_.minSize_.SetHeight(bitmap_.src_.size().y + 12);
                break;
        }
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

detail::Descriptor *Button::Desc::clone() const {
    return new Desc(*this);
}

