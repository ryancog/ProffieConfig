#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/notifier.h
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

#include <wx/window.h>

#include "utils/types.h"

#include "private/export.h"

namespace PCUI {

struct NotifierData {
    /**
     * Notify the connected Notifier of an update
     *
     * This function bitwise or's the id to allow for multi-notification
     * bitmask capability should multiple notifications happen at once.
     */
    void notify(uint32 id = 0) {
        if (mNotification) *mNotification |= id;
        else mNotification = id;
    }

private:
    friend class Notifier;
    optional<uint32> mNotification;
};

struct UI_EXPORT Notifier {
protected:
    Notifier(wxWindow *, NotifierData&);

    /**
     * Called whenever the associated data
     *
     * @param id The data-side id provided for the update.
     */
    virtual void handleNotification(uint32 id) = 0;
};

} // namespace PCUI

