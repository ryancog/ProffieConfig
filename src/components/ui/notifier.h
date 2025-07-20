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

#include <atomic>

#include <wx/window.h>

#include "utils/types.h"

#include "private/export.h"

namespace PCUI {

// TODO: There's still several possible race conditions here that
// probably deserve a second look.

struct UI_EXPORT NotifierData {
    /**
     * Notify the connected Notifier of an update
     *
     * Does not lock. Caller must lock and unlock around this and
     * data modification.
     *
     * @param id An ID to indicate what notified.
     */
    void notify(uint32 id = 0);

    std::mutex& getLock() { return mLock; }

    /**
     * Does not lock. Must be locked outside this.
     *
     * @return if any events are in flight (UI should not modify data)
     */
    bool eventsInFlight() { 
        assert(not mLock.try_lock());
        return mInFlight; 
    }

private:
    friend class Notifier;
    friend class NotifierDataProxy;
    wxWindow *mReceiver{nullptr};
    /**
     * Events in flight.
     */
    uint32 mInFlight{0};
    std::mutex mLock;
};

struct UI_EXPORT NotifierDataProxy {
    void bind(NotifierData *);
    NotifierData *data() { return mData; }

private:
    friend class Notifier;
    wxWindow *mReceiver{nullptr};
    NotifierData *mData;
};

struct UI_EXPORT Notifier {
    enum {
        ID_REBOUND = 0xFFFFFFFF,
    };

protected:
    /**
     * @param derived The window which derived this Notifier, will handle events.
     * @param data Data to bind
     */
    Notifier(wxWindow *derived, NotifierData& data);
    /**
     * @param derived The window which derived this Notifier, will handle events.
     * @param proxy Proxy to handle data binding. May be empty (but not null).
     */
    Notifier(wxWindow *derived, NotifierDataProxy& proxy);
    virtual ~Notifier();

    /**
     * Called whenever notification is received.
     *
     * Data is locked.
     *
     * @param id The data-side id provided for the update. 
     * ID_REBOUND if new notifier was bound via proxy
     */
    virtual void handleNotification(uint32 id) = 0;

    /**
     * Called when the data is unbound by proxy
     */
    virtual void handleUnbound() {}

    NotifierData *data();

private:
    NotifierData *mData{nullptr};
    NotifierDataProxy *mProxy{nullptr};
};

} // namespace PCUI

