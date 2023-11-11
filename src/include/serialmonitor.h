#pragma once

#include "threadrunner.h"

#include <wx/wx.h>

class SerialMonitor : public wxFrame {
public:
  SerialMonitor();
  ~SerialMonitor();
  static SerialMonitor* instance;

private:

  class SerialDataEvent : public wxCommandEvent {
  public:
    SerialDataEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id, const std::string& message) {
      this->SetEventType(tag);
      this->SetId(id);
      this->value = message;
    }

    std::string value;
  };
  static wxEventTypeTag<wxCommandEvent> EVT_INPUT;
  static wxEventTypeTag<wxCommandEvent> EVT_DISCON;

  ThreadRunner* deviceThread;
  ThreadRunner* listenerThread;
  ThreadRunner* writerThread;

  bool listenerRunning = false;
  bool writerRunning = false;

  wxTextCtrl* input;
  wxTextCtrl* output;

#if defined(__WXOSX__) || defined(__WXGTK__)
  int fd = 0;
#elif defined(__WXMSW__)
  HANDLE serHandle = nullptr;
#endif
  std::string sendOut;


  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
};
