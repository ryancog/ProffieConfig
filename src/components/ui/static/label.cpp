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
#include <wx/stattext.h>

#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"

using namespace pcui;

namespace {

struct Static : priv::WinBase<wxStaticText, data::String::Receiver> {
    Static(wxWindow *parent, const Label& desc) : WinBase(desc.win_) {
        if (const auto *ptr{std::get_if<wxString>(&desc.label_ )}) {
            Create(parent, wxID_ANY, *ptr);
            return;
        } 

        const auto& model{std::get<1>(desc.label_)};
        data::String::ROContext str{model};
        Create(parent, wxID_ANY, str.val());
        
        attach(model);
    }

    ~Static() override {
        detach();
    }

    void onChange() override {
        CallAfter([this, str=context<data::String>().val()]() {
            SetLabel(str);
        });
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> Label::operator()() {
    return std::make_unique<Label::Desc>(std::move(*this));
}

Label::Desc::Desc(Label&& label) :
    Label(std::move(label)) {}

wxSizerItem *Label::Desc::build(const detail::Scaffold& scaffold) const {
    auto *text{new Static(scaffold.childParent_, *this)};
    text->SetFont(style_.makeFont());

    auto *item{new wxSizerItem(text)};
    priv::apply(base_, item);

    return item;
}

