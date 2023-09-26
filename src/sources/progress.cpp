#include "progress.h"
#include "mainwindow.h"

const wxEventTypeTag<wxCommandEvent> Progress::EVT_UPDATE(wxNewEventType());

void Progress::emitEvent(int progress, wxString message) {
  wxCommandEvent event(EVT_UPDATE, wxID_ANY);
  event.SetClientData(
      new DialogInfo({
          .progress = progress,
          .message = message
      }));
  wxPostEvent(MainWindow::instance->GetEventHandler(), event);
}

void Progress::handleEvent(Progress* progress, wxCommandEvent& event) {
  Progress::DialogInfo* info = (Progress::DialogInfo*)event.GetClientData();

  if (info->progress >= 0)
    progress->Update(info->progress, info->message == "" ? progress->GetMessage() : info->message);
  else
    progress->Pulse(info->message == "" ? progress->GetMessage() : info->message);

  if (info->progress == 100)
  {
    progress->Close(true);
  }
}
