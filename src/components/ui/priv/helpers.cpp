#include "helpers.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/helpers.cpp
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

#include <cassert>

#include "utils/defer.hpp"

namespace {

// All this stuff happens on the main thread, no locking required.

// This bool tracks whether a CallAfter event to run performUpdate() has been
// sent, or if another one is needed. It does not track whether or not that
// event is needed anymore (due to early flushing), as that status could
// change, possibly multiple times, between one initially being sent and
// everything being added to the lists. (e.g. add, flush, add)
bool updateRequested{false};

std::vector<std::set<wxWindow *>> updateList;
std::unordered_map<wxWindow *, bool> showList;

void performUpdate();

void processTLW(wxWindow *);
void processChild(wxWindow *);

} // namespace

void pcui::priv::apply(const detail::ChildBase& desc, wxSizerItem *item) {
    // wxSizerItem only calls the virtual func for a window, not a sizer,
    // So I have to do this check manually here in addition to the item call.
    if (item->IsSizer()) {
        item->GetSizer()->SetMinSize(desc.minSize_);
    } else {
        // Although in most cases the window min size will be handled by
        // WinBase, there may be some that aren't, so set it here anyways.
        // Worst case it's redundant.
        item->SetMinSize(desc.minSize_);
    }

    item->SetProportion(desc.proportion_);
    item->SetBorder(desc.border_.size_);
    item->SetFlag(
        desc.border_.dirs_ | (desc.expand_ ? wxEXPAND : 0) | desc.align_
    );
}

void pcui::priv::queueShow(wxWindow *win, bool show) {
    assert(wxIsMainThread());

    showList[win] = show;
}

/*
 * This does not actually do any layout or fitting itself.
 *
 * It finds the top-level window associated with the window that caused the
 * layout to be required, adds it into the `updateList`, and queues
 * performUpdate(), if it hasn't been queued already.
 *
 * This is for the case where multiple UI updates would come in such a way that
 * one child is shown, *then* another is hidden, and where they may occupy the
 * same or similar space.
 *
 * If the layout and fitting occurred immediately, then the window may
 * unnecessarily grow, which is undesirable.
 */
void pcui::priv::layoutAndFitFor(wxWindow *win) {
    assert(wxIsMainThread());

    auto *top{win};

    std::vector<wxWindow *> hierarchy{top};

    while (not false) {
        auto *next{top->GetParent()};
        if (not next) break;

        top = next;
        hierarchy.push_back(top);

        if (next->IsTopLevel()) break;
    }

    updateList.resize(std::max(updateList.size(), hierarchy.size()));
    for (size idx{0}; idx < hierarchy.size(); ++idx) {
        // Insert in reverse so that updateList[0] is always a TLW, and the
        // greatest index is the lowest-level window, so that actual processing
        // can clearly process from lowest to highest.
        updateList[idx].insert(hierarchy[hierarchy.size() - idx - 1]);
    }

    if (not updateRequested) {
        win->CallAfter(&performUpdate);
        updateRequested = true;
    }
}

void pcui::priv::flushLayoutQueueFor(wxWindow *win) {
    assert(wxIsMainThread());

    if (updateList.empty()) return;

    // Throughout this, wxWindow::IsDescendant is used quite a bit. It seems
    // worth mentioning that the name is a little confusing, and it's checking
    // if the window passed is a descendant of `this`, *NOT* if `this` is a
    // descendant of the arg as the name would imply if read naturally.

    // Go through the showlist and process any windows which are a descendant
    // of the window we're flushing for, erasing the entry for any processed.
    for (auto iter{showList.begin()}; iter != showList.end();) {
        auto [winToShow, show]{*iter};

        if (not win->IsDescendant(winToShow)) {
            ++iter;
            continue;
        }

        winToShow->Show(show);
        iter = showList.erase(iter);
    }

    // First, process all the windows and remove the individual listings from
    // each subset as appropriate.
    for (auto iter{updateList.rbegin()};; ++iter) {
        bool isTlw{std::next(iter) == updateList.rend()};

        for (auto winIt{iter->begin()}; winIt != iter->end();) {
            if (not win->IsDescendant(*winIt)) {
                ++iter;
                continue;
            }

            if (isTlw) processTLW(*winIt);
            else processChild(*winIt);

            winIt = iter->erase(winIt);
        }

        if (isTlw) break;
    }

    // Now, remove any empty sets.
    for (auto iter{updateList.begin()}; iter != updateList.end();) {
        if (not iter->empty()) continue;

        // By the parent->child nature of the way the list is constructed,
        // once an empty set is reached, all the remaining sets must also be
        // empty. All remaining sets were children of this set windows, which,
        // since this set is empty, must have all been children of the window
        // being flushed (this set could be a child set of the flushed window
        // or the it belonged to, depending on how things fell out).
        const auto allClear{[checkIter=iter] mutable {
            ++checkIter;
            while (checkIter != updateList.end()) {
                if (not checkIter->empty()) return false;
                ++checkIter;
            }
            
            return true;
        }()};
        assert(allClear);

        iter = updateList.erase(iter, updateList.end());
    }
}

namespace {

/**
 * On every window, starting from the lowest in the hierarchy, Fit() is called.
 * Top level windows have some additional handling to not shrink.
 *
 * Fit() is used on every window as, while wxWidgets normally only has it as a
 * size set (no min size), it's the seemingly most semantically clear place for
 * windows to implement min size recalculation (Layout() is not suitable as
 * it's called during user resize, and many other times. Fit() is more
 * programmatic in nature).
 */
void performUpdate() {
    // Make sure this is always reset.
    defer { updateRequested = false; };

    // If a flush was invoked before the usual update occurred, then it's
    // possible no update is needed at all anymore.
    if (updateList.empty()) return;

    for (auto [win, show] : showList) {
        win->Show(show);
    }
    showList.clear();

    for (auto iter{updateList.rbegin()};; ++iter) {
        bool isTlw{std::next(iter) == updateList.rend()};

        for (auto *win : *iter) {
            if (isTlw) processTLW(win);
            else processChild(win);
        }

        if (isTlw) break;
    }

    updateList.clear();
}

void processTLW(wxWindow *win) {
    auto oldSize{win->GetSize()};

    win->Fit();

    if (win->GetWindowStyle() & wxRESIZE_BORDER) {
        auto fitSize{win->GetSize()};
        fitSize.IncTo(oldSize);
        win->SetSize(fitSize);
    }

    win->Layout();
}

void processChild(wxWindow *win) {
    win->Fit();
    win->Layout();
}

} // namespace

