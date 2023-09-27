#include "progress.h"
#include "mainwindow.h"
#include "wx/event.h"
#include <memory>

Progress::ProgressEvent::ProgressEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id) : wxCommandEvent(tag, id) {}
wxEventTypeTag<wxCommandEvent> Progress::EVT_UPDATE(wxNewEventType());


void Progress::emitEvent(int progress, wxString message) {
  ProgressEvent* event = new ProgressEvent(EVT_UPDATE, wxID_ANY);
  event->progress = progress;
  event->message = message;
  wxQueueEvent(MainWindow::instance->GetEventHandler(), event);
}

void Progress::handleEvent(Progress* progress, ProgressEvent* event) {
  if (event->progress >= 0) {
    progress->Update(event->progress, event->message == "" ? progress->GetMessage() : event->message);
  } else progress->Pulse(event->message == "" ? progress->GetMessage() : event->message);

  if (event->progress == 100) progress->Close(true);

}
