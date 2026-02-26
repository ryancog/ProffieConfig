#include "checkbox.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/checkbox.cpp
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

#include <wx/checkbox.h>

#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"

using namespace pcui;

namespace {

struct Control : priv::WinBase<wxCheckBox, data::Bool::Receiver> {
    Control(wxWindow *parent, const CheckBox& desc) {
        Create(
            parent,
            wxID_ANY,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.base_.align_ & wxALIGN_RIGHT
        );

        // Attach now. Virtual handler is in valid state and this is before
        // Bind can send events (the handler expects good receiver)
        attach(desc.data_);

        Bind(wxEVT_CHECKBOX, &Control::onCheck, this);
    }

    ~Control() override {
        Unbind(wxEVT_CHECKBOX, &Control::onCheck, this);
        detach();
    }

    void onCheck(wxCommandEvent& evt) {
        if (reinterpret_cast<size>(evt.GetClientData()) == 1) {
            SetValue(evt.GetInt());
            return;
        }

        auto res{processAction(std::make_unique<data::Bool::SetAction>(
            evt.IsChecked()
        ))};
        if (not res) [this, bl=context<data::Bool>()]() {
            SetValue(bl.val());
        }();
    }
    
    void onSet(bool val) override {
        auto *evt{new wxCommandEvent(wxEVT_CHECKBOX, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(1));
        evt->SetInt(val);
        wxQueueEvent(this, evt);
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> CheckBox::operator()() {
    return std::make_unique<CheckBox::Desc>(std::move(*this));
}

CheckBox::Desc::Desc(CheckBox&& data) :
    CheckBox{std::move(data)} {}

wxSizerItem *CheckBox::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold.childParent_, *this)};
    auto *item{new wxSizerItem(chk)};
    priv::apply(base_, item);
    return item;
}

