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

#include "data/number.hpp"
#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"

using namespace pcui;

namespace {

struct IntCtrl : priv::WinBase<wxSpinCtrl, data::Integer::Receiver> {
    IntCtrl(wxWindow *parent, const Stepper& desc) {
        Create(
            parent,
            wxID_ANY,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.base_.align_ & wxALIGN_RIGHT
        );

        auto params{context<data::Integer>().params()};
        SetRange(params.min_, params.max_);
        SetIncrement(params.inc_);

        attach(std::get<0>(desc.data_));

        Bind(wxEVT_SPINCTRL, &IntCtrl::onSpin, this);
    }

    ~IntCtrl() override {
        Unbind(wxEVT_SPINCTRL, &IntCtrl::onSpin, this);
        detach();
    }

    void onSpin(wxSpinEvent& evt) {
        const auto id{reinterpret_cast<size>(evt.GetClientData())};
        if (id == 1) {
            SetValue(evt.GetValue());
            return;
        } 

        if (id == 2) {
            auto params{context<data::Integer>().params()};
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
            return;
        }

        auto res{processAction(std::make_unique<data::Integer::SetAction>(
            evt.GetValue()
        ))};
        if (not res) [this, ctxt=context<data::Integer>()]() {
            SetValue(ctxt.val());
        }();
    }
    
    void onSet() override {
        auto *evt{new wxSpinEvent(wxEVT_SPINCTRL, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(1));
        evt->SetValue(context<data::Integer>().val());
        wxQueueEvent(this, evt);
    }

    void onUpdate() override {
        auto *evt{new wxSpinEvent(wxEVT_SPINCTRL, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(2));
        wxQueueEvent(this, evt);
    }
};

struct DoubleCtrl : priv::WinBase<wxSpinCtrlDouble, data::Integer::Receiver> {
    DoubleCtrl(wxWindow *parent, const Stepper& desc) {
        Create(
            parent,
            wxID_ANY,
            desc.label_,
            wxDefaultPosition,
            wxDefaultSize,
            desc.base_.align_ & wxALIGN_RIGHT
        );

        auto params{context<data::Decimal>().params()};
        SetRange(params.min_, params.max_);
        SetIncrement(params.inc_);

        attach(std::get<1>(desc.data_));

        Bind(wxEVT_SPINCTRLDOUBLE, &DoubleCtrl::onSpin, this);
    }

    ~DoubleCtrl() override {
        Unbind(wxEVT_SPINCTRLDOUBLE, &DoubleCtrl::onSpin, this);
        detach();
    }

    void onSpin(wxSpinDoubleEvent& evt) {
        const auto id{reinterpret_cast<size>(evt.GetClientData())};
        if (id == 1) {
            SetValue(evt.GetValue());
            return;
        } 

        if (id == 2) {
            auto params{context<data::Decimal>().params()};
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
            return;
        }

        auto res{processAction(std::make_unique<data::Decimal::SetAction>(
            evt.GetValue()
        ))};
        if (not res) [this, ctxt=context<data::Decimal>()]() {
            SetValue(ctxt.val());
        }();
    }
    
    void onSet() override {
        auto *evt{new wxSpinDoubleEvent(wxEVT_SPINCTRLDOUBLE, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(1));
        evt->SetValue(context<data::Decimal>().val());
        wxQueueEvent(this, evt);
    }

    void onUpdate() override {
        auto *evt{new wxSpinDoubleEvent(wxEVT_SPINCTRLDOUBLE, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(2));
        wxQueueEvent(this, evt);
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> Stepper::operator()() {
    return std::make_unique<Stepper::Desc>(std::move(*this));
}

Stepper::Desc::Desc(Stepper&& data) :
    Stepper{std::move(data)} {}

wxSizerItem *Stepper::Desc::build(const detail::Scaffold& scaffold) const {
    wxWindow *spin{nullptr};

    if (const auto *ptr{std::get_if<0>(&data_)}) {
        spin = new IntCtrl(scaffold.childParent_, *this);
    } else if (const auto *ptr{std::get_if<1>(&data_)}) {
        spin = new DoubleCtrl(scaffold.childParent_, *this);
    }

    auto *item{new wxSizerItem(spin)};
    priv::apply(base_, item);
    return item;
}

