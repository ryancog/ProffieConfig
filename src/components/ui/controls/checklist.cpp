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

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<
                     wxCheckListBox,
                     data::Selection::Receiver
                 > {
    Control(const detail::Scaffold& scaffold, const CheckList& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize
        );

        DataWindow::postCreation(scaffold, desc.win_);

        data::Selection::Context ctxt{desc.data_};
        Set(ctxt.items());
        for (auto idx{0}; idx < ctxt.selected().size(); ++idx) {
            Check(idx, ctxt.selected()[idx]);
        }

        attach(desc.data_);
        Bind(wxEVT_CHECKLISTBOX, &Control::onCheck, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onCheck(wxCommandEvent& evt) {
        auto en{this->freezeGetRealEnable()};
        defer { this->thawRealEnable(); };

        if (not en) return;
        
        auto& ch{const_cast<data::Selection&>(model<data::Selection>())};

        auto res{ch.processUIAction(
            std::make_unique<data::Selection::SelectAction>(
                evt.GetInt(), IsChecked(evt.GetInt())
            )
        )};

        if (not res) {
            auto ctxt{context<data::Selection>()};
            Check(evt.GetInt(), ctxt.selected()[evt.GetInt()]);
        }
    }

    void onSelection(uint32 idx) override {
        auto state{context<data::Selection>().selected()[idx]};
        safeCall([this, idx, state] {
            Check(idx, state);
        });
    }

    void onItems() override {
        auto items{context<data::Selection>().items()};
        safeCall([this, items] {
            Set(items);
        });
    }

    void onInsert(uint32 idx) override {
        auto ctxt{context<data::Selection>()};
        auto item{ctxt.items()[idx]};
        auto state{ctxt.selected()[idx]};
        safeCall([this, item, idx, state] {
            Insert(item, idx);
            Check(idx, state);
        });
    }

    void onRemove(uint32 idx) override {
        safeCall([this, idx] {
            Delete(idx);
        });
    }
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

