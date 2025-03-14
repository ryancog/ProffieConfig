#include "misc.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/config/configuration.h"

#include <wx/event.h>
#include <wx/string.h>
#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif
#include <wx/arrstr.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <initializer_list>

wxEventTypeTag<wxCommandEvent> Misc::EVT_MSGBOX(wxNewEventType());

const wxArrayString Misc::createEntries(const std::vector<wxString>& list) {
  wxArrayString entries;
  for (const wxString& entry : list) {
    entries.Add(entry);
  }
  return entries;
}

const wxArrayString Misc::createEntries(const std::initializer_list<wxString>& list) {
  return Misc::createEntries(static_cast<std::vector<wxString>>(list));
}
const wxArrayString Misc::createEntries(const Configuration::VMap& list) {
  wxArrayString entries;
  for (const auto& pair : list) {
    entries.Add(pair.first);
  }
  return entries;
}
