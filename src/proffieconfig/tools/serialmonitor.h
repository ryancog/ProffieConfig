#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/tools/serialmonitor.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <thread>
#include <wx/textctrl.h>

#include "../mainmenu/mainmenu.h"

class SerialMonitor : public PCUI::Frame {
public:
    SerialMonitor(MainMenu *, const string&);
    ~SerialMonitor() override;
    static SerialMonitor* instance;

private:
    class SerialDataEvent;
    static const wxEventTypeTag<SerialDataEvent> EVT_INPUT;
    static const wxEventTypeTag<SerialDataEvent> EVT_DISCON;

    enum {
        ID_SerialCommand
    };

    std::thread mDevThread;
    std::thread mListenThread;
    std::thread mWriterThread;

    wxTextCtrl *mInput;
    wxTextCtrl *mOutput;

#   if defined(__WXOSX__) or defined(__WXGTK__)
    int32_t mFd = 0;
#   elif defined(__WXMSW__)
    HANDLE mSerialHandle;
#   endif
    wxString mSendOut;
    vector<wxString> mHistory;
    int64 mHistoryIdx{0};
    bool mAutoScroll{true};

    void bindEvents();
    bool openDevice(const string&);
    void createListener();
    void createWriter();
};

class SerialMonitor::SerialDataEvent : public wxCommandEvent {
public:
    SerialDataEvent(wxEventTypeTag<SerialDataEvent> tag, int32_t id, char chr) {
        this->SetEventType(tag);
        this->SetId(id);
        this->value = chr;
    }

    char value;
};
