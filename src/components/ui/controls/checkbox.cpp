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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxCheckBox> {
    Control(const detail::Scaffold& scaffold, const CheckBox& desc) :
        bl_{desc.data_} {
        Create(
            scaffold.childParent_,
            desc.win_.id_,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.win_.base_.align_ & wxALIGN_RIGHT
        );

        postCreation(scaffold, desc.win_);

        static const auto table{[] {
            data::base::Bool::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onSet_ = data::map(&Control::onSet);
            return table;
        }()};
        observeWith(bl_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(bl_)};
        SetValue(ctxt.val());

        Bind(wxEVT_CHECKBOX, &Control::onCheck, this);
    }

    const data::base::Model *primaryModel() override {
        return &bl_;
    }

    void onCheck(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{bl_.set(evt.IsChecked())};

        if (not res) {
            auto ctxt{data::context(bl_)};
            SetValue(ctxt.val());
        }
    }
    
    void onSet() {
        safeCall([this, val=data::context(bl_).val()] {
            SetValue(val);
        });
    }

    data::base::Bool& bl_;
};

} // namespace

DescriptorPtr CheckBox::operator()() {
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

detail::Descriptor *CheckBox::Desc::clone() const {
    return new Desc(*this);
}

