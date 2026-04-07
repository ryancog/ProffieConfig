#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/selector.cpp
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

#include "ui/build.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/builder.hpp"
#include "ui/utils.hpp"

using namespace pcui;

namespace {

/**
 * This item is a bit of a hack. The idea is to insert it alongside the item
 * that is rebuilt in order to do the replacing.
 *
 * *Something* has to keep this object alive and manage it, and I suppose it
 * makes sense enough that it be the same parent as the rebuilt item, since
 * it can't be the item itself.
 */
struct TrackerDummy : detail::IDataDriven,
                      wxSizerItem,
                      data::Choice::Receiver {
    TrackerDummy(const detail::Scaffold& scaffold, const Selector& desc) :
        wxSizerItem(0, 0),
        scaffold_{scaffold},
        builder_{desc.builder_},
        sel_{desc.data_} {
        data::Choice::Context ctxt{desc.data_.choice_};

        buildAndReplace(ctxt);
        attach(desc.data_.choice_);
    }

    void preDestroyCripple() override {
        detach();
    }

    // onChoice will be called for both direct choices and indirect choices
    // as a result of unbinding (or rebinding), so it's all we need to listen
    // to.
    void onChoice() override {
        if (last_) pcui::cripple(last_);

        pcui::safeCall([this] { 
            auto ctxt{context<data::Choice>()};
            buildAndReplace(ctxt);
        });
    }

    // Build a new item and replace the old one with it, or insert a new item
    // if we don't have one yet.
    void buildAndReplace(const data::Choice::ROContext& choice) {
        data::Model *model{nullptr};

        // If we have a choice, then a vector is guaranteed to be bound. If
        // not, it doesn't matter in any case.
        if (choice.idx() != -1) {
            data::Selector::ROContext sel{sel_};
            data::Vector::ROContext vec{*sel.bound()};

            // Grab the model to build off of.
            model = &*vec.children()[choice.idx()];
        }

        auto desc{builder_(model)};
        auto *item{desc->build(scaffold_)};

        int32 insertLoc{-1};
        if (last_) {
            // If last_ holds a window, Remove will not destroy it, which
            // would be a memory leak if not cleaned up here.
            if (last_->IsWindow()) last_->GetWindow()->Destroy();

            // The old item needs to be removed from the sizer, but wxSizer
            // only provides remove by index or wxSizer *, and only provides
            // Replace for window->window and sizer->sizer.
            //
            // So, we've got to find the index of the item we're tracking
            // before being able to remove it, and then insert in a new item.
            const auto& children{scaffold_.sizer_->GetChildren()};

            // wxSizerItemList is, well, a list, so have to iterate with
            // iterators, not by index. The list deceptively provides a
            // subscript operator, but this simply iterates over the list
            // internally, so using it would be horribly inefficient.
            auto iter{children.begin()};
            for (size idx{0}; idx < children.size(); ++idx, ++iter) {
                if (*iter != last_) continue;

                // The wxSizerItemList and wxSizerItem use int, but STL types
                // use size (or ssize, or equivalent type). So a size is used
                // for `idx` to compare against the size(), but has to be
                // converted here. We're not worried about hitting the int
                // limit and it being a problem, it's just a bit confusing.
                insertLoc = static_cast<int>(idx);
                scaffold_.sizer_->Remove(static_cast<int>(idx));
                break;
            }
        }

        // Now, the wxSizerItem is deleted, if it existed.
        // Track the new item.
        last_ = item;

        if (insertLoc == -1) {
            scaffold_.sizer_->Add(item);
        } else {
            scaffold_.sizer_->Insert(insertLoc, item);
        }
    }

    // The last-built item that is currently being held in whatever sizer
    // is hold us.
    wxSizerItem *last_{nullptr};

    const detail::Scaffold scaffold_;
    const detail::DescBuilder builder_;

    // Since we're not a receiver for the selector, have to hold onto this
    // ourselves.
    const data::Selector& sel_;
};

} // namespace

std::unique_ptr<detail::Descriptor> Selector::operator()() {
    assert(builder_);
    return std::make_unique<Selector::Desc>(std::move(*this));
}

Selector::Desc::Desc(Selector&& data) :
    Selector{std::move(data)} {}

wxSizerItem *Selector::Desc::build(const detail::Scaffold& scaffold) const {
    return new TrackerDummy(scaffold, *this);
}

