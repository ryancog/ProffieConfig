// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "progress.h"

#include "mainwindow.h"

#include "wx/event.h"
#include <memory>

wxEventTypeTag<wxCommandEvent> Progress::EVT_UPDATE(wxNewEventType());

void Progress::emitEvent(int8_t progress, wxString message) {

  ProgressEvent* event = new ProgressEvent(EVT_UPDATE, wxID_ANY);
  event->progress = progress;
  event->message = message;
  wxQueueEvent(MainWindow::instance->GetEventHandler(), event);
}

void Progress::handleEvent(Progress* progress, ProgressEvent* event) {
  if (event->progress > progress->GetValue()) progress->Update(event->progress, event->message);
  else if (event->progress == -1) progress->Pulse();
}
