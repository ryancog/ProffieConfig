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

#include "data/helpers/exclusive.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/layout/priv/groupbox.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxRadioButton, data::Model::Receiver> {
    Control(
        const detail::Scaffold& scaffold,
        const wxString& label,
        data::Bool& data,
        const detail::ChildWindowBase& win
    ) {
        Create(scaffold.childParent_, wxID_ANY, label);

        postCreation(scaffold, win);

        data::Bool::Context ctxt{data};
        SetValue(ctxt.val());

        attach(data);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    friend struct Manager;
};

struct Manager : detail::DataWindow<priv::GroupBox, data::Exclusive::Receiver> {
    Manager(const detail::Scaffold& scaffold, const Radios& desc) {
        create(
            scaffold.childParent_,
            wxVERTICAL,
            desc.label_
        );

        postCreation(scaffold, desc.win_);

        auto childScaffold{scaffold};
        childScaffold.childParent_ = childParent();
        for (auto idx{0}; idx < desc.data_.data().size(); ++idx) {
            auto& bl{*desc.data_.data()[idx]};
            auto label{idx < desc.labels_.size()
                ? desc.labels_[idx]
                : "UNLABELED"
            };

            auto *radio{new Control(
                childScaffold, label, bl, desc.win_
            )};

            if (not childParent()->GetChildren().empty()) {
                sizer()->AddSpacer(5);
            }

            sizer()->Add(radio);
        }

        attach(desc.data_);
        Bind(wxEVT_RADIOBUTTON, &Manager::onSet, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onSet(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        for (auto *child : childParent()->GetChildren()) {
            if (child != evt.GetEventObject()) continue;

            auto& bl{const_cast<data::Bool&>(
                static_cast<Control *>(child)->model<data::Bool>()
            )};
            auto res{bl.processUIAction(
                std::make_unique<data::Bool::SetAction>(true)
            )};

            if (not res) {
                auto selected{model<data::Exclusive>().selected()};
                auto *child{childParent()->GetChildren()[selected]};

                static_cast<Control *>(child)->SetValue(true);
            }
            break;
        }
    }
    
    void onSelection(size idx) override {
        safeCall([this, idx] {
            auto *child{childParent()->GetChildren()[idx]};
            static_cast<wxRadioButton *>(child)->SetValue(true);
        });
    }
};

} // namespace

DescriptorPtr Radios::operator()() {
    return std::make_unique<Radios::Desc>(std::move(*this));
}

Radios::Desc::Desc(Radios&& data) :
    Radios{std::move(data)} {}

wxSizerItem *Radios::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Manager(scaffold, *this)};
    auto *item{new wxSizerItem(chk)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *Radios::Desc::clone() const {
    return new Desc(*this);
}

