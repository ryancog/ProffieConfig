// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once
#include <wx/event.h>
#include <wx/progdlg.h>

class Progress : public wxProgressDialog {
public:
  class ProgressEvent;

  void emitEvent(int8_t, wxString);
  static void handleEvent(ProgressEvent*);

  static wxEventTypeTag<wxCommandEvent> EVT_UPDATE;
  Progress(wxWindow* parent) : wxProgressDialog("", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH) {}
  bool lastWasPulse;

private:
};

class Progress::ProgressEvent : public wxCommandEvent {
public:
  ProgressEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id){
    this->SetEventType(tag);
    this->SetId(id);
  }

  Progress* progDialog;
  int8_t progress;
  wxString message;
};
