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
  static wxEventTypeTag<wxCommandEvent> EVT_RECON;

  ThreadRunner* deviceThread;
  ThreadRunner* listenerThread;
  ThreadRunner* writerThread;

  wxTextCtrl* input;
  wxTextCtrl* output;

  int fd = 0;
  bool sendOut = false;

  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
};
