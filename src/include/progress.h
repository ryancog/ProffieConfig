#pragma once
#include "wx/event.h"
#include <wx/progdlg.h>

class Progress : public wxProgressDialog {
public:
  class ProgressEvent : public wxCommandEvent {
  public:
    ProgressEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id){
      this->SetEventType(tag);
      this->SetId(id);
    }

    int progress;
    wxString message;
  };

  static void emitEvent(int, wxString);
  static void handleEvent(Progress*, ProgressEvent*);

  static wxEventTypeTag<wxCommandEvent> EVT_UPDATE;
  Progress(wxWindow* parent) : wxProgressDialog("", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE) {}
};
