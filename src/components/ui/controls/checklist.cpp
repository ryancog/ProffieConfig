#include "checklist.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/checklist.cpp
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

#include <wx/checklst.h>

#include "data/base/models/selection.hpp"
#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxCheckListBox> {
    Control(const detail::Scaffold& scaffold, const CheckList& desc) :
        sel_{desc.data_} {
        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxDefaultPosition,
            wxDefaultSize
        );

        DataWindow::postCreation(scaffold, desc.win_);

        static const auto table{[] {
            data::base::Selection::RecvTable table;
            table.onEnable_ = data::map<&DataWindow::onEnable>();
            table.onFocus_ = data::map<&DataWindow::onFocus>();
            table.onSelection_ = data::map<&Control::onSelection>();
            table.onItems_ = data::map<&Control::onItems>();
            table.onInsert_ = data::map<&Control::onInsert>();
            table.onRemove_ = data::map<&Control::onRemove>();
            return table;
        }()};
        observeWith(sel_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(sel_)};
        Set(ctxt.items());
        for (auto idx{0}; idx < ctxt.selected().size(); ++idx) {
            Check(idx, ctxt.selected()[idx]);
        }

        Bind(wxEVT_CHECKLISTBOX, &Control::onCheck, this);
    }

    const data::base::Model *primaryModel() override {
        return &sel_;
    }

    void onCheck(wxCommandEvent& evt) {
        auto en{this->freezeGetRealEnable()};
        defer { this->thawRealEnable(); };

        if (not en) return;
        
        auto res{sel_.select(
            evt.GetInt(), IsChecked(evt.GetInt())
        )};

        if (not res) {
            auto ctxt{data::context(sel_)};
            Check(evt.GetInt(), ctxt.selected()[evt.GetInt()]);
        }
    }

    void onSelection(uint32 idx) {
        auto state{data::context(sel_).selected()[idx]};
        safeCall([this, idx, state] {
            Check(idx, state);
        });
    }

    void onItems() {
        auto items{data::context(sel_).items()};
        safeCall([this, items] {
            Set(items);
        });
    }

    void onInsert(uint32 idx) {
        auto ctxt{data::context(sel_)};
        auto item{ctxt.items()[idx]};
        safeCall([this, item, idx] {
            Insert(item, idx);
        });
    }

    void onRemove(uint32 idx) {
        safeCall([this, idx] {
            Delete(idx);
        });
    }

    data::base::Selection& sel_;
};

} // namespace

DescriptorPtr CheckList::operator()() {
    return std::make_unique<CheckList::Desc>(std::move(*this));
}

CheckList::Desc::Desc(CheckList&& data) :
    CheckList{std::move(data)} {}

wxSizerItem *CheckList::Desc::build(const detail::Scaffold& scaffold) const {
    auto *ctrl{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(ctrl)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *CheckList::Desc::clone() const {
    return new Desc(*this);
}

