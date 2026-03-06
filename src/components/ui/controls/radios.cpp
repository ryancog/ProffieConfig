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
#include "ui/layout/priv/groupbox.hpp"
#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"

using namespace pcui;

namespace {

struct RadioControl : priv::WinBase<wxRadioButton, data::Model::Receiver> {
    RadioControl(wxWindow *parent, const wxString& label, data::Bool& data) {
        Create(parent, wxID_ANY, label);

        attach(data);
    }

    ~RadioControl() override {
        detach();
    }

    friend struct Control;
};

struct Control : priv::WinBase<priv::GroupBox, data::Exclusive::Receiver> {
    Control(wxWindow *parent, const Radios& desc) {
        create(
            wxVERTICAL,
            parent,
            desc.label_
        );

        attach(desc.data_);

        for (auto idx{0}; idx < desc.data_.data().size(); ++idx) {
            auto& bl{*desc.data_.data()[idx]};
            auto label{idx < desc.labels_.size()
                ? desc.labels_[idx]
                : "UNLABELED"
            };

            auto *radio{new RadioControl(childParent(), label, bl)};

            if (not childParent()->GetChildren().empty()) {
                sizer()->AddSpacer(5);
            }

            sizer()->Add(radio);
        }

        Bind(wxEVT_RADIOBUTTON, &Control::onSet, this);
    }

    ~Control() override {
        Unbind(wxEVT_RADIOBUTTON, &Control::onSet, this);
        detach();
    }

    void onSet(wxCommandEvent& evt) {
        if (reinterpret_cast<size>(evt.GetClientData()) == 1) {
            auto *child{childParent()->GetChildren()[evt.GetInt()]};
            static_cast<wxRadioButton *>(child)->SetValue(true);
            return;
        }

        for (auto *child : childParent()->GetChildren()) {
            if (child != evt.GetEventObject()) continue;

            auto res{static_cast<RadioControl *>(child)->processAction(
                std::make_unique<data::Bool::SetAction>(true)
            )};

            if (not res) {
                auto selected{model<data::Exclusive>().selected()};
                auto *child{childParent()->GetChildren()[selected]};

                static_cast<RadioControl *>(child)->SetValue(true);
            }
            break;
        }
    }
    
    void onSelection(size idx) override {
        auto *evt{new wxCommandEvent(wxEVT_RADIOBUTTON, wxID_ANY)};
        evt->SetClientData(reinterpret_cast<void *>(1));
        evt->SetInt(static_cast<int>(idx));
        wxQueueEvent(this, evt);
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> Radios::operator()() {
    return std::make_unique<Radios::Desc>(std::move(*this));
}

Radios::Desc::Desc(Radios&& data) :
    Radios{std::move(data)} {}

wxSizerItem *Radios::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold.childParent_, *this)};
    auto *item{new wxSizerItem(chk)};
    priv::apply(base_, item);
    return item;
}

