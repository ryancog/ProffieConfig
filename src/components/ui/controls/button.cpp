#include "button.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/declarative/controls/button.cpp
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

#include "ui/declarative/priv/helpers.hpp"
#include "ui/declarative/priv/winbase.hpp"
#include "ui/declarative/scaffold.hpp"

namespace {
using namespace pcui::declarative;

struct Control : priv::WinBase<wxButton, data::Generic::Receiver> {
    Control(wxWindow *parent, const Button& desc) :
        func_{desc.func_} {
        Create(
            parent,
            wxID_ANY,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.exactFit_ ? wxBU_EXACTFIT : 0
        );

        attach(desc.data_);
        Bind(wxEVT_BUTTON, &Control::onPress, this);
    }

    ~Control() override {
        Unbind(wxEVT_BUTTON, &Control::onPress, this);
        detach();
    }

    void onPress(wxCommandEvent&) {
        auto generic{context<data::Generic>()};
        if (generic.enabled()) func_();
    }

    const function<void()> func_;
};

} // namespace

std::unique_ptr<Descriptor> Button::operator()() {
    return std::make_unique<Button::Desc>(std::move(*this));
}

Button::Desc::Desc(Button&& data) :
    Button{std::move(data)} {}

wxSizerItem *Button::Desc::build(const Scaffold& parent) const {
    auto *button{new Control(parent.childParent_, *this)};
    auto *item{new wxSizerItem(button)};
    priv::apply(base_, item);
    return item;
}

