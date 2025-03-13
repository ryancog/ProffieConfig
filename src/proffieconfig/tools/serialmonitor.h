#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#if not defined(__WINDOWS__)
#include <thread>
#include "ui/controls.h"
#endif

#include "../mainmenu/mainmenu.h"

#if not defined (__WINDOWS__)
class SerialMonitor : public PCUI::Frame {
#else
class SerialMonitor {
#endif
public:
    SerialMonitor(MainMenu*);
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

    PCUI::Text *input;
    PCUI::Text *output;

    int32_t fd = 0;
    wxString sendOut;
    std::vector<wxString> history;
    ssize_t historyIdx{0};
    bool autoScroll{true};


    void BindEvents();
    void OpenDevice();
    void CreateListener();
    void CreateWriter();
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
