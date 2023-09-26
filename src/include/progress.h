#pragma once
#include "wx/event.h"
#include <wx/progdlg.h>

class Progress : public wxProgressDialog {
public:
  struct DialogInfo {
    int progress;
    wxString message;
  };

  static void emitEvent(int, wxString);
  static void handleEvent(Progress*, wxCommandEvent&);

  static const wxEventTypeTag<wxCommandEvent> EVT_UPDATE;
  Progress(wxWindow* parent) : wxProgressDialog("", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE) {}
};
