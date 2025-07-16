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

vector<string> Misc::createEntries(const std::vector<wxString>& vec) {
    vector<string> entries;
    entries.reserve(vec.size());
    for (const auto& entry : vec) {
        entries.emplace_back(entry.ToStdString());
    }
    return entries;
}

vector<string> Misc::createEntries(const std::initializer_list<wxString>& list) {
    return Misc::createEntries(static_cast<std::vector<wxString>>(list));
}

vector<string> Misc::createEntries(const Configuration::VMap& map) {
    vector<string> entries;
    entries.reserve(map.size());
    for (const auto& pair : map) {
        entries.push_back(pair.first);
    }
    return entries;
}
