#include "undo.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/undo.cpp
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

namespace Undo {

enum EventTriggers {
    NONE,
    UNDO =      1 << 0,
    REDO =      1 << 1,
    REGISTER =  1 << 2,
};

} // namespace Undo

Undo::Event::Event(int32 triggers, wxEventType eventType) : wxNotifyEvent(eventType, wxID_ANY), mTriggers(triggers) {}

wxEvent *Undo::Event::Clone() const { return new Event(*this); }

bool Undo::Event::isUndo() const { return mTriggers & UNDO; }
bool Undo::Event::isRedo() const { return mTriggers & REDO; }
bool Undo::Event::isRegister() const { return mTriggers & REGISTER; }

wxDEFINE_EVENT(Undo::EVT_ACTION, Undo::Event);
wxDEFINE_EVENT(Undo::EVT_AT_SAVED, Undo::Event);
wxDEFINE_EVENT(Undo::EVT_LEAVE_SAVED, Undo::Event);

Undo::Action::Action(wxString name, ActionFunc perform, ActionFunc revert, any data) :
    mDoPerform(std::move(perform)), mDoRevert(std::move(revert)), mName(std::move(name)), mData(std::move(data)) {}

void Undo::Action::perform() {
    if (not mDoPerform) return;
    mDoPerform(mData);
}

void Undo::Action::revert() {
    if (not mDoRevert) return;
    mDoRevert(mData);
}

const wxString& Undo::Action::name() const { return mName; }

Undo::Handler::Handler(wxEvtHandler *evtHandler, int32 maxDepth) : mMaxDepth(maxDepth), mEvtHandler(evtHandler) {}

optional<wxString> Undo::Handler::canUndo() const { return mIdx >= 0 ? optional<wxString>{mActions[mIdx].name()} : nullopt; }
optional<wxString> Undo::Handler::canRedo() const { return mIdx + 1 < mActions.size() ? optional<wxString>{mActions[mIdx + 1].name()} : nullopt; };
bool Undo::Handler::isSaved() const { return mSavedIdx == mIdx; }

void Undo::Handler::undo() {
    if (not canUndo()) return;

    bool wasOnSaved{isSaved()};

    mActions[mIdx].revert();
    mIdx--;

    wxPostEvent(mEvtHandler, Event(UNDO, EVT_ACTION));
    if (wasOnSaved) wxPostEvent(mEvtHandler, Event(UNDO, EVT_LEAVE_SAVED));
    else if (isSaved()) wxPostEvent(mEvtHandler, Event(UNDO, EVT_AT_SAVED));
}
void Undo::Handler::redo() {
    if (not canRedo()) return;

    bool wasOnSaved{isSaved()};

    mIdx++;
    mActions[mIdx].perform();

    wxPostEvent(mEvtHandler, Event(REDO, EVT_ACTION));
    if (wasOnSaved) wxPostEvent(mEvtHandler, Event(REDO, EVT_LEAVE_SAVED));
    else if (isSaved()) wxPostEvent(mEvtHandler, Event(REDO, EVT_AT_SAVED));
}

void Undo::Handler::registerAction(Action&& action, bool perform) {
    bool wasOnSaved{isSaved()};

    if (mActions.size() == mMaxDepth) {
        mActions.pop_front();
        mIdx--;
        mSavedIdx--;
    }

    mIdx++;
    if (mIdx < mActions.size()) mActions.erase(std::next(mActions.begin(), mIdx), mActions.end());
    mActions.emplace_back(std::move(action));

    if (perform) mActions.back().perform();

    wxPostEvent(mEvtHandler, Event(REGISTER, EVT_ACTION));
    if (wasOnSaved) wxPostEvent(mEvtHandler, Event(REGISTER, EVT_LEAVE_SAVED));
}

void Undo::Handler::flagAsSaved() {
    mSavedIdx = mIdx;

    wxPostEvent(mEvtHandler, Event(NONE, EVT_AT_SAVED));
}

