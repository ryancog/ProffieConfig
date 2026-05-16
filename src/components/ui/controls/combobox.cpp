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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxComboBox> {
    Control(const detail::Scaffold& scaffold, const ComboBox& desc) :
        str_{desc.data_} {
        wxString initial;

        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            0
        );

        postCreation(scaffold, desc.win_);

        if (not desc.hint_.empty())
            SetHint(desc.hint_);

        Set(desc.defaults_);

        static const auto table{[] {
            data::base::String::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onChange_ = data::map(&Control::onChange);
            table.onMove_ = data::map(&Control::onMove);
            return table;
        }()};
        amend(str_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetValue(data::context(str_).val());

        Bind(wxEVT_TEXT, &Control::onText, this);
    }

    const data::base::Model *primaryModel() override {
        return &str_;
    }

    void onText(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{str_.change(GetValue().ToStdString(), GetInsertionPoint())};

        if (not res) {
            auto ctxt{data::context(str_)};
            ChangeValue(ctxt.val());
            SetInsertionPoint(static_cast<long>(ctxt.pos()));
        }
    }

    void onChange() {
        safeCall([this, val=data::context(str_).val()] {
            ChangeValue(val);
        });
    }

    void onMove() {
        safeCall([this, pos=data::context(str_).pos()] {
            SetInsertionPoint(static_cast<long>(pos));
        });
    }

    data::base::String& str_;
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

