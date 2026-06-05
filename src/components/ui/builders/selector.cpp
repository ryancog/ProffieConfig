#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/builders/selector.cpp
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

#include <wx/panel.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/priv/tracker.hpp"
#include "ui/types.hpp"

using namespace pcui;
using namespace pcui::builders;

namespace {

/**
 * This item is a bit of a hack. The idea is to insert it alongside the item
 * that is rebuilt in order to do the replacing.
 *
 * *Something* has to keep this object alive and manage it, and I suppose it
 * makes sense enough that it be the same parent as the rebuilt item, since
 * it can't be the item itself.
 */
struct SelectorTracker : priv::Tracker, data::Receiver {
    SelectorTracker(detail::Scaffold scaffold, const Selector& desc) :
        Tracker(scaffold),
        builder_{desc.builder_},
        sel_{desc.data_} {
        static const auto table{[] {
            data::base::Selector::RecvTable table;
            table.onRebound_ = data::map<&SelectorTracker::onRebound>();
            return table;
        }()};
        observeWith(sel_, table);

        static const auto choiceTable{[] {
            data::base::Choice::RecvTable table;
            table.onChoice_ = data::map<&SelectorTracker::onChoice>();
            return table;
        }()};
        observeWith(sel_.choice(), choiceTable);

        activate();
    }

    void onActivate() override {
        rebuild();
    }

    void onChoice() {
        rebuild();
    }

    void onRebound() {
        rebuild();
    }

    void rebuild() {
        auto ctxt{data::context(sel_.choice())};

        data::base::Model *model{nullptr};

        // If we have a choice, then a vector is guaranteed to be bound. If
        // not, it doesn't matter in any case.
        if (ctxt.idx() != -1)
            // Grab the model to build off of.
            model = data::context(sel_).selected();

        // If a UI element has already been built and the model to be built off
        // is the same, then there's nothing to do, just leave things alone.
        if (isBuilt() and lastModel_ == model) return;

        // Set this before doRebuild() so it can be used from the descriptor builder.
        lastModel_ = model;

        doRebuild();
    }

    DescriptorPtr buildDescriptor() override {
        return builder_(lastModel_);
    }

    const data::base::Selector& sel_;
    data::base::Model *lastModel_{nullptr};
    const std::function<Selector::DescBuilder> builder_;
};

} // namespace

DescriptorPtr Selector::operator()() {
    assert(builder_);
    return std::make_unique<Selector::Desc>(std::move(*this));
}

Selector::Desc::Desc(Selector&& data) :
    Selector{std::move(data)} {}

wxSizerItem *Selector::Desc::build(const detail::Scaffold& scaffold) const {
    return new SelectorTracker(scaffold, *this);
}

detail::Descriptor *Selector::Desc::clone() const {
    return new Desc(*this);
}

