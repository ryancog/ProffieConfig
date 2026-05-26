#include "vecstack.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/builders/vecstack.cpp
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

#include <list>

#include <wx/event.h>
#include <wx/sizer.h>

#include "data/context.hpp"
#include "data/receiver.hpp"
#include "ui/build.hpp"
#include "ui/detail/helpers.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/types.hpp"
#include "ui/utils.hpp"

using namespace pcui;
using namespace pcui::builders;

namespace {

struct Layout : wxBoxSizer, data::Receiver {
    Layout(const detail::Scaffold& scaffold, const VecStack& desc) :
        wxBoxSizer(desc.orient_),
        vec_{desc.data_} {
        builder_ = desc.builder_;
        separator_ = desc.separator_;

        childScaffold_ = scaffold;
        childScaffold_.sizer_ = this;

        if (desc.empty_)
            emptyElem_ = desc.empty_->build(childScaffold_);

        static const auto table{[] {
            data::base::Vector::RecvTable table;
            table.onInsert_ = data::map(&Layout::onInsert);
            table.preRemove_ = data::map(&Layout::preRemove);
            table.onSwap_ = data::map(&Layout::onSwap);
            return table;
        }()};
        amend(vec_, table);

        activate();
    }

    void onActivate() override {
        auto ctxt{data::context(vec_)};

        if (emptyElem_) {
            Add(emptyElem_);
            Show(0UL, ctxt.children().empty());
        }

        auto children{ctxt.children()};
        for (auto idx{0}; idx < children.size(); ++idx)
            onInsert(idx);
    }

    void onInsert(size pos) {
        auto mapIter{map_.emplace(
            map_.end(),
            data::context(vec_).children()[pos].get(),
            nullptr
        )};

        // In all GUI operations, `pos` is captured solely for the GUI
        // functions. The data is accessed separately, as it may be out of
        // sync.
        safeCall([this, pos, mapIter] {
            // To lock map_
            std::lock_guard scopeLock(pMutex);

            assert(mapIter->second == nullptr);
            if (mapIter->first) {
                mapIter->second = builder_(*mapIter->first)->build(childScaffold_);
            } else {
                // Even if the model has died, the GUI hasn't caught up, so
                // create a dummy in the interim.
                mapIter->second = new wxSizerItem(0, 0);
            }

            if (emptyElem_) {
                Hide(0UL);
            }

            if (separator_ and AreAnyItemsShown()) {
                // For insertions anywhere but beginning, since, with a separator,
                // the sizerPos() computes after where the separator goes.
                int insertPos{sizerPos(pos) - 1};
                if (pos == 0)
                    // Both will prepend, separator goes first, so it will be after
                    // the item.
                    insertPos = 0;

                Insert(insertPos, separator_->build(childScaffold_));
            }

            Insert(sizerPos(pos), mapIter->second);

            if (auto *win{GetContainingWindow()})
                detail::layoutAndFitFor(win);
        });
    }

    void preRemove(size pos) {
        auto ctxt{data::context(vec_)};

        auto *toRemove{ctxt.children()[pos].get()};

        // Find the mapping for the model, and store the iter so the safeCall
        // knows what to erase.
        auto iter{map_.begin()};
        for (; iter != map_.end(); ++iter) {
            if (iter->first != toRemove) continue;

            // Make sure the item is crippled since the model it was built on
            // is about to die.
            if (iter->second)
                cripple(iter->second);

            // In case the remove happens before the GUI insert can.
            iter->first = nullptr;

            break;
        }
        assert(iter != map_.end());

        safeCall([this, pos, iter] {
            // To lock map_
            std::lock_guard scopeLock(pMutex);

            wxWindow *toDelete{nullptr};

            // This logic is similar to that of Selector's buildAndReplace
            if (iter->second->IsWindow())
                toDelete = iter->second->GetWindow();
            else if (iter->second->IsSizer())
                iter->second->GetSizer()->DeleteWindows();

            Remove(sizerPos(pos));

            if (separator_ and AreAnyItemsShown()) {
                // Remove the separator from the next element that moved "up"
                int removePos{sizerPos(pos)};

                // Notice here the end is checked w/ removePos & GetChildren(),
                // but the beginning is checked w/ pos. This is intentional to
                // account for both separators and empty elem, or lack thereof.
                //
                // If this is the last item, there is no next separator.
                if (removePos == GetChildren().size()) {
                    // Even if there isn't a next separator to remove, maybe
                    // there's a previous one that needs removal (if this isn't
                    // the first.)
                    if (pos > 0) --removePos;

                    // Otherwise, let GetItem fail.
                }

                if (auto *item{GetItem(removePos)}) {
                    wxWindow *separatorToDelete{nullptr};

                    if (item->IsWindow())
                        separatorToDelete = item->GetWindow();
                    else if (item->IsSizer())
                        item->GetSizer()->DeleteWindows();

                    Remove(removePos);

                    if (separatorToDelete)
                        separatorToDelete->Destroy();
                }
            }

            if (toDelete)
                toDelete->Destroy();

            if (emptyElem_ and GetChildren().size() == 1) {
                Show(0UL);
            }

            if (auto *win{GetContainingWindow()})
                detail::layoutAndFitFor(win);

            // This is the end of the model<->item lifecycle, remove from map.
            map_.erase(iter);
        });
    }

    // TODO: On at least macOS, when things are swapped buttons don't trigger
    // again until the mouse is moved.
    void onSwap(size pos) {
        safeCall([this, pos] {
            // First, detach the "lower" item and bring it up above the "upper"
            // one.
            auto *item{DetachItem(sizerPos(pos + 1))};
            Insert(sizerPos(pos), item);

            if (separator_) {
                // If there's a separator, now detach the "upper" (but now below
                // what was formerly "lower") item and move it to where "lower"
                // used to be (properly in between separators).
                auto *item{DetachItem(sizerPos(pos) + 1)};
                Insert(sizerPos(pos + 1), item);
            }

            wxBoxSizer::Layout();
        });
    }

    int sizerPos(size pos) {
        if (separator_) pos *= 2;
        if (emptyElem_) pos +=1;

        return static_cast<int>(pos);
    }

    const data::base::Vector& vec_;
    detail::Scaffold childScaffold_;
    VecStack::Builder builder_;
    DescriptorPtr separator_;
    wxSizerItem *emptyElem_{nullptr};

    // Because the GUI might not (and realistically won't be during quick
    // changes) be synced, keep a mapping of items and what model they were
    // built off.
    //
    // These use the receiver mutex for locking.
    std::list<std::pair<data::base::Model *, wxSizerItem *>> map_;
};

} // namespace

DescriptorPtr VecStack::operator()() {
    return std::make_unique<VecStack::Desc>(std::move(*this));
}

VecStack::Desc::Desc(VecStack&& data) : VecStack{std::move(data)} {}

wxSizerItem *VecStack::Desc::build(const detail::Scaffold& scaffold) const {
    auto *item{new wxSizerItem(new Layout(scaffold, *this))};
    detail::apply(base_, item);
    return item;
}

detail::Descriptor *VecStack::Desc::clone() const {
    return new Desc(*this);
}

