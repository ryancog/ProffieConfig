#include "stepper.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/stepper.cpp
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

#include <wx/spinctrl.h>
#include <wx/colour.h>

#include "data/number.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct IntCtrl : detail::DataWindow<wxSpinCtrl, data::Integer::Receiver> {
    IntCtrl(const detail::Scaffold& scaffold, const Stepper& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            desc.win_.base_.align_ & wxALIGN_RIGHT
        );

        data::Integer::Context ctxt{std::get<0>(desc.data_)};
        SetRange(ctxt.params().min_, ctxt.params().max_);
        SetIncrement(ctxt.params().inc_);
        SetValue(ctxt.val());

        postCreation(scaffold, desc.win_);

        attach(std::get<0>(desc.data_));
        Bind(wxEVT_SPINCTRL, &IntCtrl::onSpin, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onSpin(wxSpinEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& num{const_cast<data::Integer&>(model<data::Integer>())};
        auto res{num.processUIAction(
            std::make_unique<data::Integer::SetAction>(evt.GetValue())
        )};

        if (not res) {
            auto ctxt{context<data::Integer>()};
            SetValue(ctxt.val());
        }
    }
    
    void onSet() override {
        const auto val{context<data::Integer>().val()};
        safeCall([this, val] {
            SetValue(val);
        });
    }

    void onUpdate() override {
        auto params{context<data::Integer>().params()};
        safeCall([this, params] {
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
        });
    }
};

struct DoubleCtrl : detail::DataWindow<wxSpinCtrlDouble, data::Integer::Receiver> {
    DoubleCtrl(const detail::Scaffold& scaffold, const Stepper& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            desc.win_.base_.align_ & wxALIGN_RIGHT
        );

        postCreation(scaffold, desc.win_);

        data::Decimal::Context ctxt{std::get<1>(desc.data_)};
        SetRange(ctxt.params().min_, ctxt.params().max_);
        SetIncrement(ctxt.params().inc_);
        SetValue(ctxt.val());

        attach(std::get<1>(desc.data_));
        Bind(wxEVT_SPINCTRLDOUBLE, &DoubleCtrl::onSpin, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onSpin(wxSpinDoubleEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& dec{const_cast<data::Decimal&>(model<data::Decimal>())};
        auto res{dec.processUIAction(
            std::make_unique<data::Decimal::SetAction>(evt.GetValue())
        )};

        if (not res) {
            auto ctxt{context<data::Decimal>()};
            SetValue(ctxt.val());
        }
    }
    
    void onSet() override {
        const auto val{context<data::Decimal>().val()};
        safeCall([this, val] {
            SetValue(val);
        });
    }

    void onUpdate() override {
        auto params{context<data::Decimal>().params()};
        safeCall([this, params] {
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
        });
    }
};

} // namespace

DescriptorPtr Stepper::operator()() {
    return std::make_unique<Stepper::Desc>(std::move(*this));
}

Stepper::Desc::Desc(Stepper&& data) :
    Stepper{std::move(data)} {}

wxSizerItem *Stepper::Desc::build(const detail::Scaffold& scaffold) const {
    wxWindow *spin{nullptr};

    if (const auto *ptr{std::get_if<0>(&data_)}) {
        spin = new IntCtrl(scaffold, *this);
    } else if (const auto *ptr{std::get_if<1>(&data_)}) {
        spin = new DoubleCtrl(scaffold, *this);
    }

    auto *item{new wxSizerItem(spin)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Stepper::Desc::clone() const {
    return new Desc(*this);
}

