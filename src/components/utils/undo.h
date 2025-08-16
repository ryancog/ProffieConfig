#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/undo.h
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

#include <utils/types.h>

#include "utils_export.h"

namespace Undo {

using ActionFunc = function<void(any&)>;

struct UTILS_EXPORT Event : wxNotifyEvent {
    Event(int32 triggers, wxEventType);

    [[nodiscard]] wxEvent *Clone() const override;

    // Check triggers
    [[nodiscard]] bool isUndo() const;
    [[nodiscard]] bool isRedo() const;
    [[nodiscard]] bool isRegister() const;

private:
    int32 mTriggers;
};

// Triggered on any undo/redo action.
// Can use `isX()` to check what action occurred.
UTILS_EXPORT wxDECLARE_EVENT(EVT_ACTION, Event);

// Reached the action in the chain which is saved, or
// the current action was flagged as the current saved version.
//
// Not sent on Handler init.
UTILS_EXPORT wxDECLARE_EVENT(EVT_AT_SAVED, Event);

// Left the action in the chain which is saved and/or 
// an undo/redo occurred.
UTILS_EXPORT wxDECLARE_EVENT(EVT_LEAVE_SAVED, Event);

struct UTILS_EXPORT Action {
    Action(wxString name, ActionFunc perform, ActionFunc revert, any data = {});

    void perform();
    void revert();
    [[nodiscard]] const wxString& name() const;

private:
    ActionFunc mDoPerform;
    ActionFunc mDoRevert;

    wxString mName;
    any mData;
};

struct UTILS_EXPORT Handler {
    Handler(wxEvtHandler *, int32 maxDepth = 1024);

    // Return name of action that can be undone/redone if it exists.
    [[nodiscard]] optional<wxString> canUndo() const;
    [[nodiscard]] optional<wxString> canRedo() const;
    [[nodiscard]] bool isSaved() const;

    void undo();
    void redo();

    void registerAction(Action&&, bool perform = true);

    void flagAsSaved();

private:
    int32 mMaxDepth;
    int32 mIdx{-1};
    int32 mSavedIdx{-1};
    deque<Action> mActions;

    wxEvtHandler *mEvtHandler;
};

} // namespace Undo

