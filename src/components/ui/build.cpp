#include "build.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/build.cpp
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
#include <wx/wupdlock.h>

#include "ui/frame.hpp"
#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"

void pcui::build(wxWindow *win, const DescriptorPtr& desc) {
    wxWindowUpdateLocker lock(win);

    teardown(win);

    auto *parent{win};
    if (dynamic_cast<pcui::Frame *>(parent)) {
        parent = new wxPanel(win);
    }

    if (not desc) return;

    // In many cases this means there's a kind of unnecessary sizer in a sizer,
    // but this is the most straightforward way to allow base (essentially just
    // sizer) properties to work at the top level.
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    detail::Scaffold scaffold{
        .childParent_=parent,
        .sizer_=sizer,
    };

    auto *item{desc->build(scaffold)};
    sizer->Add(item);

    // Immediately flush any show updates that were caused from building the
    // new UI elements, which could otherwise be seen by the user as
    // "flickering" as things update between the event loop servicing the UI.
    priv::flushShowQueueFor(win);

    // Also, clear out any layout requests that were generated from the build,
    // they're redundant since layout will be processed here shortly.
    priv::discardLayoutsFor(win);

    // The sizer SetSizerAndFit, contrary to its name, does not SetSizer() and
    // Fit(). Instead, it calls some special functions to do something similar.
    //
    // Since this setup places more logic into Fit() and overrides it to behave
    // in the more expected way (such that a naive SetSizer() and Fit()) would
    // work as the actual SetSizerAndFit(), doing calcs and setting min), it
    // must actually be called directly.
    //
    // TL;DR SetSizerAndFit() does not call Fit().
    parent->SetSizer(sizer);

    // Grab old size for Layout() test below.
    const auto oldSize{parent->GetVirtualSize()};

    // In any case, we actually don't want Fit() on the parent, but rather on
    // the win. The parent might be an intermediary panel!!
    win->Fit();

    // If the old and new items resulted in exactly the same size, layout has
    // to be triggered manually (AutoLayout from the sizer doesn't occur w/o
    // resize event).
    //
    // Since layout isn't cheap afaics, actually check that size is the same.
    // I'm not sure if this is the best way to do things though, or if the
    // Layout() responsibility should be pushed to the caller. For now, it's
    // unexpected that build() doesn't always result in a clean new setup, so
    // do this for consistency. 
    if (oldSize == parent->GetVirtualSize()) {
        priv::layoutAndFitFor(win);
        priv::flushLayoutQueueFor(win);
    }
}

void pcui::teardown(wxWindow *win) {
    wxWindowUpdateLocker lock(win);

    assert(parent);

    // First, delete all children of the active sizer, and the sizer itself.
    // That should clear out most windows in most cases.
    if (win->GetSizer()) {
        win->GetSizer()->DeleteWindows();
        win->SetSizer(nullptr, true);
    }

    // Then, check for any other children. They cannot be unconditionally
    // deleted because there's things like toolbars (and probably other things)
    // that need to be considered.
    //
    // There might be a cleaner and/or more efficient way to do this, but we
    // can't just iterate over GetChildren() directly, because the iterators
    // will be invalidated as child Destroy() is called.
    auto iter{win->GetChildren().begin()};
    while (iter != win->GetChildren().end()) {
        if (not win->IsClientAreaChild(*iter)) {
            ++iter;
            continue;
        }

        (*iter)->Destroy();
        iter = win->GetChildren().begin();
    }
}

