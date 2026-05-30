#include "choice.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/builders/choice.cpp
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
#include "data/receiver.hpp"
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
struct ChoiceTracker : priv::Tracker, data::Receiver {
    ChoiceTracker(detail::Scaffold scaffold, const Choice& desc) :
        Tracker(scaffold),
        builder_{desc.builder_},
        choice_{desc.data_} {
        static const auto choiceTable{[] {
            data::base::Choice::RecvTable table;
            table.onChoice_ = data::map(&ChoiceTracker::onChoice);
            return table;
        }()};
        observeWith(choice_, choiceTable);

        activate();
    }

    void onActivate() override {
        doRebuild();
    }

    void onChoice() {
        doRebuild();
    }

    DescriptorPtr buildDescriptor() override {
        auto ctxt{data::context(choice_)};
        return builder_(ctxt.idx());
    }

    const data::base::Choice& choice_;
    const std::function<Choice::DescBuilder> builder_;
};

} // namespace

DescriptorPtr Choice::operator()() {
    assert(builder_);
    return std::make_unique<Choice::Desc>(std::move(*this));
}

Choice::Desc::Desc(Choice&& data) :
    Choice{std::move(data)} {}

wxSizerItem *Choice::Desc::build(const detail::Scaffold& scaffold) const {
    return new ChoiceTracker(scaffold, *this);
}

detail::Descriptor *Choice::Desc::clone() const {
    return new Desc(*this);
}

