#include "misc.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/config/configuration.h"

#include <wx/event.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <initializer_list>

const wxEventTypeTag<wxCommandEvent> Misc::EVT_MSGBOX(wxNewEventType());

wxArrayString Misc::createEntries(const std::vector<wxString>& vec) {
    wxArrayString entries;
    for (const wxString& entry : vec) {
        entries.Add(entry);
    }
    return entries;
}

wxArrayString Misc::createEntries(const std::initializer_list<wxString>& list) {
    return Misc::createEntries(static_cast<std::vector<wxString>>(list));
}

wxArrayString Misc::createEntries(const Configuration::VMap& map) {
    wxArrayString entries;
    for (const auto& pair : map) {
        entries.Add(pair.first);
    }
    return entries;
}
