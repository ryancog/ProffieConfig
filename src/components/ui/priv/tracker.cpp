#include "tracker.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/tracker.cpp
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

#include "ui/build.hpp"
#include "ui/detail/helpers.hpp"
#include "ui/utils.hpp"

using namespace pcui::priv;

Tracker::Tracker(detail::Scaffold scaffold) :
    wxSizerItem(0, 0),
    mScaffold{scaffold} {}

void Tracker::doRebuild() {
    // Access to last_ is safe here because inside buildAndReplace it will
    // only be accessed once a model context is acquired. Inside handlers,
    // the model is guaranteed to be locked.
    if (mLast)
        cripple(mLast);

    safeCall([this] { 
        buildAndReplace();
    });
}

bool Tracker::isBuilt() const {
    return mLast != nullptr;
}

void Tracker::buildAndReplace() {
    auto desc{buildDescriptor()};
    auto *item{desc->build(mScaffold)};

    int32 insertLoc{-1};
    if (mLast) {
        wxWindow *toDelete{nullptr};

        // If last_ holds a window, Remove will not destroy it, which
        // would be a memory leak if not cleaned up explicitly.
        //
        // Don't destroy it yet though so that the sizer child list isn't
        // modified.
        if (mLast->IsWindow())
            toDelete = mLast->GetWindow();
        // Similarly, removing a sizer won't delete the windows in the
        // sizer. This is just a quirk of wxWidget's ownership model The
        // parent window owns its children, not the sizer, so destroying
        // the sizer doesn't normally remove the windows.
        else if (mLast->IsSizer()) {
            mLast->GetSizer()->DeleteWindows();
        }

        // The old item needs to be removed from the sizer, but wxSizer
        // only provides remove by index or wxSizer *, and only provides
        // Replace for window->window and sizer->sizer.
        //
        // So, we've got to find the index of the item we're tracking
        // before being able to remove it, and then insert in a new item.
        const auto& children{mScaffold.sizer_->GetChildren()};

        // wxSizerItemList is, well, a list, so have to iterate with
        // iterators, not by index. The list deceptively provides a
        // subscript operator, but this simply iterates over the list
        // internally, so using it would be horribly inefficient.
        auto iter{children.begin()};
        for (size idx{0}; idx < children.size(); ++idx, ++iter) {
            if (*iter != mLast) continue;

            // The wxSizerItemList and wxSizerItem use int, but STL types
            // use size (or ssize, or equivalent type). So a size is used
            // for `idx` to compare against the size(), but has to be
            // converted here. We're not worried about hitting the int
            // limit and it being a problem, it's just a bit confusing.
            insertLoc = static_cast<int>(idx);
            mScaffold.sizer_->Remove(static_cast<int>(idx));
            break;
        }

        // Now the list has been walked and sizer item removed, destroy
        // window.
        if (toDelete)
            toDelete->Destroy();
    }

    // Now, the wxSizerItem is deleted, if it existed.
    // Track the new item.
    mLast = item;

    if (insertLoc == -1) {
        mScaffold.sizer_->Add(item);
    } else {
        mScaffold.sizer_->Insert(insertLoc, item);
    }

    // And don't forget to layout w/ the new insertion into the sizer
    // list.
    detail::layoutAndFitFor(mScaffold.childParent_);
}

