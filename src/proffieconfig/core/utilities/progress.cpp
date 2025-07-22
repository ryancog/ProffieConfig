#include "progress.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <utility>

#include <wx/event.h>
#include <wx/utils.h>

const wxEventTypeTag<ProgressEvent> Progress::EVT_UPDATE(wxNewEventType());

void Progress::emitEvent(int8_t progress, wxString message) {
    auto *event{new ProgressEvent(EVT_UPDATE, wxID_ANY)};
    event->progress = progress;
    event->message = std::move(message);
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
        event->progDialog->Pulse(event->message);
    }

    if (event->progress == 100) {
        event->progDialog->Update(0, "");
        wxYield();
        event->progDialog->Close(true);
    }
}
