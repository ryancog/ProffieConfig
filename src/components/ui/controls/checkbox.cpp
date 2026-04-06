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

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxCheckBox, data::Bool::Receiver> {
    Control(const detail::Scaffold& scaffold, const CheckBox& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.win_.base_.align_ & wxALIGN_RIGHT
        );

        postCreation(scaffold, desc.win_);

        data::Bool::Context ctxt{desc.data_};
        SetValue(ctxt.val());

        attach(desc.data_);
        Bind(wxEVT_CHECKBOX, &Control::onCheck, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onCheck(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& bl{const_cast<data::Bool&>(model<data::Bool>())};
        auto res{bl.processUIAction(std::make_unique<data::Bool::SetAction>(
            evt.IsChecked()
        ))};

        if (not res) {
            auto ctxt{context<data::Bool>()};
            SetValue(ctxt.val());
        }
    }
    
    void onSet() override {
        const auto val{context<data::Bool>().val()};
        safeCall([this, val] {
            SetValue(val);
        });
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> CheckBox::operator()() {
    return std::make_unique<CheckBox::Desc>(std::move(*this));
}

CheckBox::Desc::Desc(CheckBox&& data) :
    CheckBox{std::move(data)} {}

wxSizerItem *CheckBox::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(chk)};
    detail::apply(win_.base_, item);
    return item;
}

