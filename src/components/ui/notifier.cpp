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

void PCUI::NotifierData::notify(uint32 id) {
    assert(not mLock.try_lock());
    if (not mReceiver) return;

    ++mInFlight;
    if (wxThread::IsMain()) {
        NotifierEvent evt{EVT_NOTIFY, id, this};
        wxPostEvent(mReceiver, evt);
    } else {
        auto *evt{new NotifierEvent(EVT_NOTIFY, id, this)};
        wxQueueEvent(mReceiver, evt);
    }
}

void PCUI::NotifierDataProxy::bind(NotifierData *data) {
    if (mData == data) return;

    mData->mLock.lock();
    mData->mReceiver = nullptr;
    mData->mLock.unlock();

    mData = data;
    if (mData) {
        std::scoped_lock newLock{mData->mLock};
        mData->mReceiver = mReceiver;
        mData->notify(Notifier::ID_REBOUND);
    } else {
        if (wxThread::IsMain()) {
            NotifierEvent evt{EVT_UNBOUND, 0, nullptr};
            wxPostEvent(mReceiver, evt);
        } else {
            auto *evt{new NotifierEvent(EVT_UNBOUND, 0, nullptr)};
            wxQueueEvent(mReceiver, evt);
        }
    }
}

PCUI::Notifier::Notifier(wxWindow *derived, NotifierData& data) :
    mData{&data} {
    std::scoped_lock scopeLock{data.mLock};
    data.mReceiver = derived;
    data.mInFlight = 0;

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
}

PCUI::Notifier::Notifier(wxWindow *derived, NotifierDataProxy& proxy) :
    mProxy{&proxy} {
    proxy.mReceiver = derived;
    if (proxy.mData) {
        std::scoped_lock scopeLock{proxy.mData->mLock};
        proxy.mData->mReceiver = derived;
        proxy.mData->mInFlight = 0;
    }

    derived->Bind(EVT_NOTIFY, [this](NotifierEvent& evt) {
        if (this->data() != evt.getData()) return;
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
    derived->Bind(EVT_UNBOUND, [this](NotifierEvent& evt) {
        if (this->data() != evt.getData()) return;
        std::scoped_lock scopeLock{evt.getData()->mLock};
        handleNotification(evt.getID());
        --evt.getData()->mInFlight;
    });
}

PCUI::Notifier::~Notifier() {
    if (mData) {
        std::scoped_lock scopeLock{mData->mLock};
        mData->mReceiver = nullptr;
    }
    if (mProxy) {
        mProxy->mReceiver = nullptr;
        if (mProxy->mData) {
            std::scoped_lock scopeLock{mProxy->mData->mLock};
            mProxy->mData->mReceiver = nullptr;
        }
    }
}

PCUI::NotifierData *PCUI::Notifier::data() {
    if (mProxy) return mProxy->mData;
    return mData;
}


