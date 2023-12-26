// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#if defined(__WXOSX__) || defined(__WXGTK__)
#include "core/utilities/threadrunner.h"
#endif
#include <wx/wx.h>

#include "mainmenu/mainmenu.h"

class SerialMonitor : public wxFrame {
public:
  SerialMonitor(MainMenu*);
  static SerialMonitor* instance;

#if defined(__WXOSX__) || defined(__WXGTK__)
  ~SerialMonitor();

private:
  class SerialDataEvent;
  static wxEventTypeTag<wxCommandEvent> EVT_INPUT;
  static wxEventTypeTag<wxCommandEvent> EVT_DISCON;

  enum {
      ID_SerialCommand
  };

  ThreadRunner* deviceThread{nullptr};
  ThreadRunner* listenerThread{nullptr};
  ThreadRunner* writerThread{nullptr};

  bool listenerRunning{false};
  bool writerRunning{false};

  wxTextCtrl* input;
  wxTextCtrl* output;

  int32_t fd = 0;
  wxString sendOut;


  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
#endif // OSX or GTK
};

#if defined(__WXOSX__) || defined(__WXGTK__)
class SerialMonitor::SerialDataEvent : public wxCommandEvent {
public:
  SerialDataEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id, const wxString& message) {
    this->SetEventType(tag);
    this->SetId(id);
    this->value = message;
  }

  wxString value;
};
#endif
