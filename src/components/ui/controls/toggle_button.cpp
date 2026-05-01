#include "toggle_button.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/toggle_button.cpp
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

#include <wx/tglbtn.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxToggleButton> {
    Control(const detail::Scaffold& scaffold, const ToggleButton& desc) :
        data_{desc.data_} {
        Create(
            scaffold.childParent_,
            wxID_ANY,
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
        amend(data_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetValue(data::context(data_).val());

        Bind(wxEVT_TOGGLEBUTTON, &Control::onToggle, this);
    }

    const data::base::Model *primaryModel() override {
        return &data_;
    }

    void onToggle(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{data_.set(evt.IsChecked())};

        if (not res) {
            auto ctxt{data::context(data_)};
            SetValue(ctxt.val());
        }
    }

    void onSet() {
        safeCall([this, val=data::context(data_).val()] {
            SetValue(val);
        });
    }

    data::base::Bool& data_;
};

} // namespace

DescriptorPtr ToggleButton::operator()() {
    return std::make_unique<ToggleButton::Desc>(std::move(*this));
}

ToggleButton::Desc::Desc(ToggleButton&& data) :
    ToggleButton{std::move(data)} {}

wxSizerItem *ToggleButton::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(chk)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *ToggleButton::Desc::clone() const {
    return new Desc(*this);
}

