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
    NotifierEvent(wxEventType eventType, uint32 id, NotifierData *data) :
        wxEvent(wxID_ANY, eventType), mID{id}, mData{data} {}

    [[nodiscard]] uint32 getID() const { return mID; }
    [[nodiscard]] NotifierData *getData() const { return mData; }

    virtual wxEvent *Clone() const { return new NotifierEvent(*this); }

private:
    uint32 mID;
    NotifierData *mData;
};

const wxEventTypeTag<NotifierEvent> EVT_NOTIFY{wxNewEventType()};
const wxEventTypeTag<NotifierEvent> EVT_UNBOUND{wxNewEventType()};

} // namespace PCUI

PCUI::NotifierData::~NotifierData() {
    // Handle this dying before the notifier does.
    if (mNotifier) mNotifier->mData = nullptr;
}


void PCUI::NotifierData::notify(uint32 id) {
    std::scoped_lock scopeLock{mLock};
    if (not mNotifier) return;

    ++mInFlight;
    if (wxThread::IsMain()) {
        NotifierEvent evt{EVT_NOTIFY, id, this};
        wxPostEvent(mNotifier->mDerived, evt);
    } else {
        auto *evt{new NotifierEvent(EVT_NOTIFY, id, this)};
        wxQueueEvent(mNotifier->mDerived, evt);
    }
}

bool PCUI::NotifierData::eventsInFlight() {
    std::scoped_lock scopeLock{mLock};
    return mInFlight; 
}


PCUI::NotifierDataProxy::~NotifierDataProxy() {
    // Handle this dying before the notifier does.
    if (mData) mData->mNotifier = nullptr;
    if (mNotifier) mNotifier->mProxy = nullptr;
}

void PCUI::NotifierDataProxy::bind(NotifierData *data) {
    if (mData == data) return;

    if (mData) {
        mData->mLock.lock();
        mData->mNotifier = nullptr;
        mData->mLock.unlock();
    }

    mData = data;
    if (mData) {
        std::scoped_lock newLock{mData->mLock};
        mData->mNotifier = mNotifier;
        mData->notify(Notifier::ID_REBOUND);
    } else {
        if (wxThread::IsMain()) {
            NotifierEvent evt{EVT_UNBOUND, 0, nullptr};
            wxPostEvent(mNotifier->mDerived, evt);
        } else {
            auto *evt{new NotifierEvent(EVT_UNBOUND, 0, nullptr)};
            wxQueueEvent(mNotifier->mDerived, evt);
        }
    }
}

PCUI::Notifier::Notifier(wxWindow *derived, NotifierData& data) {
    create(derived, data);
}

PCUI::Notifier::Notifier(wxWindow *derived, NotifierDataProxy& proxy) {
    create(derived, proxy);
}

void PCUI::Notifier::create(wxWindow *derived, NotifierData& data) {
    assert(mDerived == nullptr);

    mData = &data;
    mDerived = derived;
    std::scoped_lock scopeLock{data.mLock};
    data.mNotifier = this;
    data.mInFlight = 0;

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
}

void PCUI::Notifier::create(wxWindow *derived, NotifierDataProxy& proxy) {
    assert(mDerived == nullptr);

    mProxy = &proxy;
    mDerived = derived;
    proxy.mNotifier = this;
    if (proxy.mData) {
        std::scoped_lock scopeLock{proxy.mData->mLock};
        proxy.mData->mNotifier = this;
        proxy.mData->mInFlight = 0;
    }

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        if (this->data() != evt.getData()) return;
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
    derived->Bind(EVT_UNBOUND, [this](NotifierEvent& evt) {
        handleUnbound();
    });
}

PCUI::Notifier::~Notifier() {
    if (mData) {
        std::scoped_lock scopeLock{mData->mLock};
        mData->mNotifier = nullptr;
    }
    if (mProxy) {
        mProxy->mNotifier = nullptr;
        if (mProxy->mData) {
            std::scoped_lock scopeLock{mProxy->mData->mLock};
            mProxy->mData->mNotifier = nullptr;
        }
    }
}

PCUI::NotifierData *PCUI::Notifier::data() {
    if (mProxy) return mProxy->mData;
    return mData;
}


