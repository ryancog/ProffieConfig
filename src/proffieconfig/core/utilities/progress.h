#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

#ifdef _WIN32
#undef wxProgressDialog
#include <wx/progdlg.h>
#define wxProgressDialog wxGenericProgressDialog
#include <wx/generic/progdlgg.h>
#else
#include <wx/progdlg.h>
#endif

class ProgressEvent;

class Progress : public wxProgressDialog {
public:

  void emitEvent(int8_t, wxString);
  static void handleEvent(ProgressEvent*);

  static const wxEventTypeTag<ProgressEvent> EVT_UPDATE;
  Progress(wxWindow* parent) : wxProgressDialog("", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH) {}
  bool lastWasPulse;

private:
};

class ProgressEvent : public wxCommandEvent {
public:
    ProgressEvent(wxEventType type, int32_t id) :
        wxCommandEvent(type, id) {}

    Progress* progDialog;
    int8_t progress;
    wxString message;
};
