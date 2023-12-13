// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "threadrunner.h"
#include <wx/wx.h>

class SerialMonitor : public wxFrame {
public:
  SerialMonitor();
  static SerialMonitor* instance;

#if defined(__WXOSX__) || defined(__WXGTK__)
  ~SerialMonitor();

private:
  class SerialDataEvent : public wxCommandEvent {
  public:
    SerialDataEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id, const wxString& message) {
      this->SetEventType(tag);
      this->SetId(id);
      this->value = message;
    }

    wxString value;
  };
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

#if defined(__WXOSX__) || defined(__WXGTK__)
  int32_t fd = 0;
#elif defined(__WXMSW__)
  HANDLE serHandle{nullptr};
#endif // if OSX/GTK elif MSW
  wxString sendOut;


  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
#endif // OSX or GTK
};
