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

  ThreadRunner* deviceThread{nullptr};
  ThreadRunner* listenerThread{nullptr};
  ThreadRunner* writerThread{nullptr};

  bool listenerRunning{false};
  bool writerRunning{false};

  wxTextCtrl* input;
  wxTextCtrl* output;

#if defined(__WXOSX__) || defined(__WXGTK__)
  int fd = 0;
#elif defined(__WXMSW__)
  HANDLE serHandle{nullptr};
#endif
  wxString sendOut;


  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
#endif
};
