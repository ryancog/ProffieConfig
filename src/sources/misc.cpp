#include "misc.h"

#include "defines.h"

#include <wx/msgdlg.h>
#include <wx/arrstr.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <cstring>
#include <iostream>
#include <initializer_list>
#include <memory>
#include <string>
#include <fstream>

#ifdef __WXOSX__
char Misc::path[];
#endif

wxEventTypeTag<wxCommandEvent> Misc::EVT_MSGBOX(wxNewEventType());

Misc::numEntry* Misc::createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal) {
  wxBoxSizer* numEntryBox = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
  wxSpinCtrl* numEntry = new wxSpinCtrl(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal);
  numEntryBox->Add(text, MENUITEMFLAGS.Center());
  numEntryBox->Add(numEntry, MENUITEMFLAGS.Border(wxRIGHT, 5));

  Misc::numEntry* returnVal = new Misc::numEntry();
  returnVal->box = numEntryBox;
  returnVal->num = numEntry;

  return returnVal;
}
Misc::numEntryDouble* Misc::createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal) {
    wxBoxSizer* numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrlDouble *numEntry = new wxSpinCtrlDouble(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal, 0.1);
    numEntryBox->Add(text, MENUITEMFLAGS.Center());
    numEntryBox->Add(numEntry, MENUITEMFLAGS.Border(wxRIGHT, 5));

    Misc::numEntryDouble* returnVal = new Misc::numEntryDouble;
    returnVal->box = numEntryBox;
    returnVal->num = numEntry;

    return returnVal;
}

const wxArrayString Misc::createEntries(std::vector<wxString> list) {
    wxArrayString entries;
    for (const wxString& entry : list)
      entries.Add(entry);

    return entries;
}
const wxArrayString Misc::createEntries(std::initializer_list<wxString> list) {
    return Misc::createEntries(static_cast<std::vector<wxString>>(list));
}
