#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

#ifdef __WINDOWS__
#undef wxProgressDialog
#include <wx/progdlg.h>
#define wxProgressDialog wxGenericProgressDialog
#include <wx/generic/progdlgg.h>
#else
#include <wx/progdlg.h>
#endif

class Progress : public wxProgressDialog {
public:
  class ProgressEvent;

  void emitEvent(int8_t, wxString);
  static void handleEvent(ProgressEvent*);

  static const wxEventTypeTag<wxCommandEvent> EVT_UPDATE;
  Progress(wxWindow* parent) : wxProgressDialog("", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH) {}
  bool lastWasPulse;

private:
};

class Progress::ProgressEvent : public wxCommandEvent {
public:
  ProgressEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id) {
    SetEventType(tag);
    SetId(id);
  }

  Progress* progDialog;
  int8_t progress;
  wxString message;
};
