#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/tracker.hpp
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

#include <wx/sizer.h>

#include "ui/detail/scaffold.hpp"
#include "ui/types.hpp"

namespace pcui::priv {

/**
 * This item is a bit of a hack. The idea is to insert it alongside the item
 * that is rebuilt in order to do the replacing.
 *
 * *Something* has to keep this object alive and manage it, and I suppose it
 * makes sense enough that it be the same parent as the rebuilt item, since
 * it can't be the item itself.
 */
struct Tracker : wxSizerItem {
    Tracker(detail::Scaffold);

protected:
    /**
     * Called by derived to perform a rebuild.
     */
    void doRebuild();

    /**
     * Is there a UI built currently?
     */
    [[nodiscard]] bool isBuilt() const;

    virtual DescriptorPtr buildDescriptor() = 0;

private:
    /**
     * Build a new item and replace the old one with it, or insert a new item
     * if we don't have one yet.
     */
    void buildAndReplace();

    /**
     * The last-built item that is currently being held in whatever sizer
     * is holding us.
     */
    wxSizerItem *mLast{nullptr};

    /**
     * Scaffold to build into.
     */
    const detail::Scaffold mScaffold;
};

} // namespace pcui::priv

