#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#if not defined(__WINDOWS__)
#include <thread>
#include <wx/textctrl.h>
#endif

#include "../mainmenu/mainmenu.h"

#if not defined (__WINDOWS__)
class SerialMonitor : public PCUI::Frame {
#else
class SerialMonitor {
#endif
public:
    SerialMonitor(MainMenu *, const string&);
    ~SerialMonitor();
    static SerialMonitor* instance;

#if not defined(__WINDOWS__)
private:
    class SerialDataEvent;
    static wxEventTypeTag<SerialDataEvent> EVT_INPUT;
    static wxEventTypeTag<SerialDataEvent> EVT_DISCON;

    enum {
        ID_SerialCommand
    };

    std::thread devThread;
    std::thread listenThread;
    std::thread writerThread;

    wxTextCtrl *input;
    wxTextCtrl *output;

    int32_t fd = 0;
    wxString sendOut;
    vector<wxString> history;
    ssize_t historyIdx{0};
    bool autoScroll{true};


    void bindEvents();
    void openDevice(const string&);
    void createListener();
    void createWriter();
#endif // OSX or GTK
};

#if not defined(__WINDOWS__)
class SerialMonitor::SerialDataEvent : public wxCommandEvent {
public:
    SerialDataEvent(wxEventTypeTag<SerialDataEvent> tag, int32_t id, char chr) {
        this->SetEventType(tag);
        this->SetId(id);
        this->value = chr;
    }

    char value;
};
#endif
