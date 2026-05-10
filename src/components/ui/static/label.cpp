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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Static : detail::DataWindow<wxStaticText> {
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

            activate();
            return;
        } 

        str_ = &std::get<1>(desc.label_).get();

        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );
        
        postCreation(scaffold, desc.win_);
        setup();

        static const auto table{[] {
            data::base::String::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onChange_ = data::map(&Static::onChange);
            return table;
        }()};
        amend(*str_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        if (str_)
            SetLabel(data::context(*str_).val());
    }

    const data::base::Model *primaryModel() override {
        return str_;
    }

    void onChange() {
        safeCall([this, str=data::context(*str_).val()]() {
            SetLabel(str);
        });
    }

    color::Dynamic color_;
    const data::base::String *str_{nullptr};
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

