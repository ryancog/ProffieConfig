#include "combobox.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/combobox.cpp
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

#include <wx/combobox.h>
#include <wx/sizer.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxComboBox, data::String::Receiver> {
    Control(const detail::Scaffold& scaffold, const ComboBox& desc) {
        wxString initial;

        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            0
        );

        postCreation(scaffold, desc.win_);

        Set(desc.defaults_);

        data::String::Context ctxt{desc.data_};
        SetValue(ctxt.val());
        attach(desc.data_);

        Bind(wxEVT_TEXT, &Control::onText, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onText(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& str{const_cast<data::String&>(model<data::String>())};

        auto res{str.processUIAction(
            std::make_unique<data::String::ChangeAction>(
                GetValue().ToStdString(), GetInsertionPoint()
            )
        )};

        if (not res) {
            auto ctxt{context<data::String>()};
            ChangeValue(ctxt.val());
            SetInsertionPoint(static_cast<long>(ctxt.pos()));
        }
    }

    void onChange() override {
        const auto val{context<data::String>().val()};
        safeCall([this, val] {
            ChangeValue(val);
        });
    }

    void onMove() override {
        const auto pos{context<data::String>().pos()};
        safeCall([this, pos] {
            SetInsertionPoint(static_cast<long>(pos));
        });
    }
};

} // namespace

DescriptorPtr ComboBox::operator()() {
    return std::make_unique<ComboBox::Desc>(std::move(*this));
}

ComboBox::Desc::Desc(ComboBox&& data) :
    ComboBox{std::move(data)} {}

wxSizerItem *ComboBox::Desc::build(const detail::Scaffold& scaffold) const {
    auto *ctrl{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(ctrl)};

    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *ComboBox::Desc::clone() const {
    return new Desc(*this);
}

