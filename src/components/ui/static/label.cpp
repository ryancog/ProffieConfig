#include "label.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/static/label.cpp
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

#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/stattext.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "wx/event.h"

using namespace pcui;

namespace {

struct Static : detail::DataWindow<wxStaticText, data::String::Receiver> {
    Static(const detail::Scaffold& scaffold, const Label& desc) {
        const auto style{desc.win_.base_.align_};

        const auto setup{[&] {
            if (desc.color_) {
                color_ = desc.color_;

                const auto onColorChange{[this]() {
                    SetForegroundColour(color_.color());
                }};

                Bind(
                    wxEVT_SYS_COLOUR_CHANGED,
                    [onColorChange](wxSysColourChangedEvent& evt) {
                        evt.Skip();
                        onColorChange();
                    }
                );
                onColorChange();
            }

            SetOwnFont(desc.font_.makeFont());
            Wrap(desc.wrapWidth_);
        }};

        if (const auto *ptr{std::get_if<wxString>(&desc.label_ )}) {
            Create(
                scaffold.childParent_,
                wxID_ANY,
                *ptr,
                wxDefaultPosition,
                wxDefaultSize,
                style
            );

            postCreation(scaffold, desc.win_);
            setup();

            return;
        } 

        const auto& model{std::get<1>(desc.label_)};
        data::String::ROContext str{model};
        Create(
            scaffold.childParent_,
            wxID_ANY,
            str.val(),
            wxDefaultPosition,
            wxDefaultSize,
            style
        );
        
        postCreation(scaffold, desc.win_);
        setup();

        attach(model);
    }

    ~Static() override {
        detach();
    }

    void onChange() override {
        safeCall([this, str=context<data::String>().val()]() {
            SetLabel(str);
        });
    }

    color::Dynamic color_;
};

} // namespace

DescriptorPtr Label::operator()() {
    return std::make_unique<Label::Desc>(std::move(*this));
}

Label::Desc::Desc(Label&& label) :
    Label(std::move(label)) {}

wxSizerItem *Label::Desc::build(const detail::Scaffold& scaffold) const {
    auto *text{new Static(scaffold, *this)};

    auto *item{new wxSizerItem(text)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Label::Desc::clone() const {
    return new Desc(*this);
}

