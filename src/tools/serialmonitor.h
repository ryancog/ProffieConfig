// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <thread>
#include <semaphore>

#if !defined(__WINDOWS__)
#include "ui/pctextctrl.h"
#endif

#include "mainmenu/mainmenu.h"

class SerialMonitor : public wxFrame {
public:
  SerialMonitor(MainMenu*);
  static SerialMonitor* instance;

#if !defined(__WINDOWS__)
  ~SerialMonitor();

private:
  class SerialDataEvent;
  static wxEventTypeTag<wxCommandEvent> EVT_INPUT;
  static wxEventTypeTag<wxCommandEvent> EVT_DISCON;

  enum {
      ID_SerialCommand
  };

  std::thread devThread;
  std::thread listenThread;
  std::thread writerThread;

  pcTextCtrl* input;
  pcTextCtrl* output;

  int32_t fd = 0;
  wxString sendOut;


  void BindEvents();
  void OpenDevice();
  void CreateListener();
  void CreateWriter();
#endif // OSX or GTK
};

#if !defined(__WINDOWS__)
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
