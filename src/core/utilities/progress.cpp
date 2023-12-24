// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "progress.h"

#include "editor/editorwindow.h"

#include <wx/event.h>

wxEventTypeTag<wxCommandEvent> Progress::EVT_UPDATE(wxNewEventType());

void Progress::emitEvent(int8_t progress, wxString message) {

  ProgressEvent* event = new ProgressEvent(EVT_UPDATE, wxID_ANY);
  event->progress = progress;
  event->message = message;
  wxQueueEvent(EditorWindow::instance->GetEventHandler(), event);
}

void Progress::handleEvent(Progress* progress, ProgressEvent* event) {
  static bool lastWasPulse{false};
  if (event->progress > progress->GetValue() || (lastWasPulse && event->progress != -1)) {
    lastWasPulse = false;
    progress->Update(event->progress, event->message);
  }
  else if (event->progress == -1) {
    lastWasPulse = true;
    progress->Pulse();
  }
}
