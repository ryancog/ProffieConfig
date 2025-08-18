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

#include <mutex>

#include <wx/window.h>

#include "utils/types.h"

#include "ui_export.h"

namespace PCUI {

// TODO: There's still several possible race conditions here that
// probably deserve a second look.

struct NotifyReceiver;
struct NotifierProxy;

struct UI_EXPORT Notifier {
    virtual ~Notifier();

    /**
     * Notify the connected Notifier of an update
     *
     * Does not lock. Caller must lock and unlock around this and
     * data modification.
     *
     * @param id An ID to indicate what notified.
     */
    void notify(uint32 id = 0);

    std::recursive_mutex& getLock() { return mLock; }

    /**
     * Does not lock. Must be locked outside this.
     *
     * @return if any events are in flight (UI should not modify data)
     */
    bool eventsInFlight() ;

private:
    friend struct NotifyReceiver;
    friend struct NotifierProxy;
    NotifyReceiver *mNotifier{nullptr};
    NotifierProxy *mProxy{nullptr};

    /**
     * Events in flight.
     */
    uint32 mInFlight{0};
    std::recursive_mutex mLock;
};

struct UI_EXPORT NotifierProxy {
    virtual ~NotifierProxy();
    
    void bind(Notifier *);
    Notifier *data() { return mData; }

private:
    friend struct NotifyReceiver;
    NotifyReceiver *mNotifier{nullptr};
    Notifier *mData{nullptr};
};

struct UI_EXPORT NotifyReceiver {
    enum {
        ID_REBOUND = 0xFFFFFFFF,
    };

    /**
     * Sync notifications with GUI
     */
    static inline void sync() { wxYield(); }

protected:
    NotifyReceiver() = default;

    /**
     * @param derived The window which derived this Notifier, will handle events.
     * @param data Data to bind
     */
    NotifyReceiver(wxWindow *derived, Notifier& data);
    /**
     * @param derived The window which derived this Notifier, will handle events.
     * @param proxy Proxy to handle data binding. May be empty (but not null).
     */
    NotifyReceiver(wxWindow *derived, NotifierProxy& proxy);

    // Late ctors
    void create(wxWindow *derived, Notifier& data);
    void create(wxWindow *derived, NotifierProxy& proxy);

    /**
     * Call handleNotification(ID_REBOUND) or handleUnbound() to
     * set up initial derived state accordingly.
     */
    void initializeNotifier();

    virtual ~NotifyReceiver();

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

    [[nodiscard]] Notifier *data();
    [[nodiscard]] NotifierProxy *proxy() { return mProxy; }

private:
    friend Notifier;
    friend NotifierProxy;
    wxWindow *mDerived{nullptr};
    Notifier *mData{nullptr};
    NotifierProxy *mProxy{nullptr};
};

} // namespace PCUI

