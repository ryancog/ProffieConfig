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

#include "data/base/models/number.hpp"
#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct IntCtrl : detail::DataWindow<wxSpinCtrl> {
    IntCtrl(const detail::Scaffold& scaffold, const Stepper& desc) :
        int_{std::get<0>(desc.data_).get()} {
        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            0
        );

        postCreation(scaffold, desc.win_);

        static const auto table{[] {
            data::base::Integer::RecvTable table;
            table.onEnable_ = data::map<&DataWindow::onEnable>();
            table.onFocus_ = data::map<&DataWindow::onFocus>();
            table.onSet_ = data::map<&IntCtrl::onSet>();
            table.onUpdate_ = data::map<&IntCtrl::onUpdate>();
            return table;
        }()};
        observeWith(int_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(int_)};
        SetRange(ctxt.params().min_, ctxt.params().max_);
        SetIncrement(ctxt.params().inc_);
        SetValue(ctxt.val());

        Bind(wxEVT_SPINCTRL, &IntCtrl::onSpin, this);
    }

    const data::base::Model *primaryModel() override {
        return &int_;
    }

    void onSpin(wxSpinEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{int_.set(evt.GetValue())};

        if (not res)
            SetValue(data::context(int_).val());
    }
    
    void onSet() {
        safeCall([this, val=data::context(int_).val()] {
            SetValue(val);
        });
    }

    void onUpdate() {
        safeCall([this, params=data::context(int_).params()] {
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
        });
    }

    data::base::Integer& int_;
};

struct DoubleCtrl : detail::DataWindow<wxSpinCtrlDouble> {
    DoubleCtrl(const detail::Scaffold& scaffold, const Stepper& desc) :
        dec_{std::get<1>(desc.data_).get()} {
        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            0
        );

        postCreation(scaffold, desc.win_);

        static const auto table{[] {
            data::base::Decimal::RecvTable table;
            table.onEnable_ = data::map<&DataWindow::onEnable>();
            table.onFocus_ = data::map<&DataWindow::onFocus>();
            table.onSet_ = data::map<&DoubleCtrl::onSet>();
            table.onUpdate_ = data::map<&DoubleCtrl::onUpdate>();
            return table;
        }()};
        observeWith(dec_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(dec_)};
        SetRange(ctxt.params().min_, ctxt.params().max_);
        SetIncrement(ctxt.params().inc_);
        SetValue(ctxt.val());

        Bind(wxEVT_SPINCTRLDOUBLE, &DoubleCtrl::onSpin, this);
    }

    const data::base::Model *primaryModel() override {
        return &dec_;
    }

    void onSpin(wxSpinDoubleEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{dec_.set(evt.GetValue())};

        if (not res)
            SetValue(data::context(dec_).val());
    }
    
    void onSet() {
        safeCall([this, val=data::context(dec_).val()] {
            SetValue(val);
        });
    }

    void onUpdate() {
        safeCall([this, params=data::context(dec_).params()] {
            SetRange(params.min_, params.max_);
            SetIncrement(params.inc_);
        });
    }
    
    data::base::Decimal& dec_;
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

