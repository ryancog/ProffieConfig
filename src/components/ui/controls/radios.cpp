#include "radios.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/radios.cpp
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

#include <wx/radiobut.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/layout/priv/groupbox.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxRadioButton> {
    Control(
        const detail::Scaffold& scaffold,
        const wxString& label,
        data::base::Bool& data,
        const detail::ChildWindowBase& win
    ) : bl_{data} {
        Create(scaffold.childParent_, wxID_ANY, label);

        postCreation(scaffold, win);

        static const auto table{[] {
            data::base::Bool::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onSet_ = data::map(&Control::onSet);
            return table;
        }()};
        amend(bl_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetValue(data::context(bl_).val());

        Bind(wxEVT_RADIOBUTTON, &Control::onButton, this);
    }

    const data::base::Model *primaryModel() override {
        return &bl_;
    }

    void onButton(wxCommandEvent& evt) {
        if (not evt.GetInt()) return;

        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        wxRadioButton *last{GetFirstInGroup()};
        for (; last != nullptr; last = last->GetNextInGroup()) {
            // At least on macOS, this can be sent even when the button is
            // already selected, so don't exclude `this` from the check.
            if (last->GetValue())
                break;
        }
        assert(last != nullptr);

        auto res{bl_.set(true)};

        if (not res)
            last->SetValue(true);
    }

    void onSet() {
        safeCall([this, val=data::context(bl_).val()] {
            SetValue(val);
        });
    }
    
    data::base::Bool& bl_;
};

struct Box : detail::Window<priv::GroupBox> {
    Box(const detail::Scaffold& scaffold, const Radios& desc) {
        create(
            scaffold.childParent_,
            desc.win_.id_,
            wxVERTICAL,
            desc.label_
        );

        postCreation(scaffold, desc.win_);

        auto childScaffold{scaffold};
        childScaffold.childParent_ = childParent();

        auto ctxt{data::context(desc.data_)};
        for (size idx{0}; idx < ctxt.num(); ++idx) {
            auto& bl{ctxt[idx]};
            auto label{idx < desc.labels_.size()
                ? desc.labels_[idx]
                : "UNLABELED"
            };

            auto *radio{new Control(
                childScaffold, label, bl, desc.win_
            )};

            if (not childParent()->GetChildren().empty()) {
                // TODO: Should this be interControlSpacing()?
                sizer()->AddSpacer(5);
            }

            sizer()->Add(radio);
        }

        activate();
    }
};

} // namespace

DescriptorPtr Radios::operator()() {
    return std::make_unique<Radios::Desc>(std::move(*this));
}

Radios::Desc::Desc(Radios&& data) :
    Radios{std::move(data)} {}

wxSizerItem *Radios::Desc::build(const detail::Scaffold& scaffold) const {
    auto *box{new Box(scaffold, *this)};

    auto *item{new wxSizerItem(box)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Radios::Desc::clone() const {
    return new Desc(*this);
}

