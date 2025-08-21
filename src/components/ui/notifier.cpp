#include "notifier.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/notifier.cpp
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

#include <wx/event.h>

namespace PCUI {

class NotifierEvent : public wxEvent {
public:
    NotifierEvent(wxEventType eventType, uint32 id, Notifier *data) :
        wxEvent(wxID_ANY, eventType), mID{id}, mData{data} {}

    [[nodiscard]] uint32 getID() const { return mID; }
    [[nodiscard]] Notifier *getData() const { return mData; }

    [[nodiscard]] wxEvent *Clone() const override { return new NotifierEvent(*this); }

private:
    uint32 mID;
    Notifier *mData;
};

const wxEventTypeTag<NotifierEvent> EVT_NOTIFY{wxNewEventType()};
const wxEventTypeTag<NotifierEvent> EVT_UNBOUND{wxNewEventType()};

} // namespace PCUI

PCUI::Notifier::~Notifier() {
    // Handle this dying (shortly) before the notifier does.
    if (mReceiver) mReceiver->mData = nullptr;
    // Handle this dying behind a proxy (allowed)
    if (mProxy) mProxy->bind(nullptr);
}


void PCUI::Notifier::notify(uint32 id) {
    std::scoped_lock scopeLock{mLock};
    if (not mReceiver) return;

    ++mInFlight;
    if (wxThread::IsMain()) {
        NotifierEvent evt{EVT_NOTIFY, id, this};
        wxPostEvent(mReceiver->mDerived, evt);
    } else {
        auto *evt{new NotifierEvent(EVT_NOTIFY, id, this)};
        wxQueueEvent(mReceiver->mDerived, evt);
    }
}

bool PCUI::Notifier::eventsInFlight() {
    std::scoped_lock scopeLock{mLock};
    return mInFlight; 
}


PCUI::NotifierProxy::~NotifierProxy() {
    // Handle this dying before the notifier does.
    if (mData) {
        mData->mReceiver = nullptr;
        mData->mProxy = nullptr;
    }
    if (mNotifier) mNotifier->mProxy = nullptr;
}

void PCUI::NotifierProxy::bind(Notifier *data) {
    if (mData == data) return;

    if (mData) {
        mData->mLock.lock();
        mData->mReceiver = nullptr;
        mData->mProxy = nullptr;
        mData->mLock.unlock();
    }

    mData = data;
    if (mData) {
        std::scoped_lock newLock{mData->mLock};
        assert(not mData->linked());

        mData->mProxy = this;
        mData->mReceiver = mNotifier;
        mData->mInFlight = 0;
        if (mNotifier) mNotifier->mDerived->DeletePendingEvents();
        mData->notify(NotifyReceiver::ID_REBOUND);
    } else if (mNotifier) {
        if (wxThread::IsMain()) {
            NotifierEvent evt{EVT_UNBOUND, 0, nullptr};
            wxPostEvent(mNotifier->mDerived, evt);
        } else {
            auto *evt{new NotifierEvent(EVT_UNBOUND, 0, nullptr)};
            wxQueueEvent(mNotifier->mDerived, evt);
        }
    }
}

PCUI::NotifyReceiver::NotifyReceiver(wxWindow *derived, Notifier& data) {
    assert(not data.linked());
    create(derived, data);
}

PCUI::NotifyReceiver::NotifyReceiver(wxWindow *derived, NotifierProxy& proxy) {
    create(derived, proxy);
}

void PCUI::NotifyReceiver::create(wxWindow *derived, Notifier& data) {
    assert(mDerived == nullptr);

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });

    mData = &data;
    mDerived = derived;
    std::scoped_lock scopeLock{data.mLock};
    data.mReceiver = this;
    data.mInFlight = 0;
}

void PCUI::NotifyReceiver::create(wxWindow *derived, NotifierProxy& proxy) {
    assert(mDerived == nullptr);

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        if (this->data() != evt.getData()) return;
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
    derived->Bind(EVT_UNBOUND, [this](NotifierEvent&) {
        handleUnbound();
    });

    mProxy = &proxy;
    mDerived = derived;
    proxy.mNotifier = this;
    if (proxy.mData) {
        std::scoped_lock scopeLock{proxy.mData->mLock};
        proxy.mData->mReceiver = this;
        proxy.mData->mInFlight = 0;
    }
}

void PCUI::NotifyReceiver::initializeNotifier() {
    assert(mDerived != nullptr and (mProxy != nullptr or mData != nullptr));

    if (mProxy) {
        if (mProxy->mData) {
            std::scoped_lock scopeLock{mProxy->mData->getLock()};
            handleNotification(ID_REBOUND);
        } else {
            handleUnbound();
        }
    } else if (mData) {
        std::scoped_lock scopeLock{mData->getLock()};
        handleNotification(ID_REBOUND);
    }
}

PCUI::NotifyReceiver::~NotifyReceiver() {
    if (mData) {
        std::scoped_lock scopeLock{mData->mLock};
        mData->mReceiver = nullptr;
    }
    if (mProxy) {
        mProxy->mNotifier = nullptr;
        if (mProxy->mData) {
            std::scoped_lock scopeLock{mProxy->mData->mLock};
            mProxy->mData->mReceiver = nullptr;
        }
    }
}

PCUI::Notifier *PCUI::NotifyReceiver::data() {
    if (mProxy) return mProxy->mData;
    return mData;
}


