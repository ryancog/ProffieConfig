#include "progress.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/event.h>

wxEventTypeTag<wxCommandEvent> Progress::EVT_UPDATE(wxNewEventType());

void Progress::emitEvent(int8_t progress, wxString message) {
  ProgressEvent* event = new ProgressEvent(EVT_UPDATE, wxID_ANY);
  event->progress = progress;
  event->message = message;
  event->progDialog = this;
  wxQueueEvent(GetParent()->GetEventHandler(), event);
}

void Progress::handleEvent(ProgressEvent* event) {
  if (event->progress > event->progDialog->GetValue() || (event->progDialog->lastWasPulse && event->progress != -1)) {
    event->progDialog->lastWasPulse = false;
    event->progDialog->Update(event->progress, event->message);
  }
  else if (event->progress == -1) {
    event->progDialog->lastWasPulse = true;
    event->progDialog->Pulse();
  }

  if (event->progress == 100) {
    event->progDialog->Close(true);
  }
}
