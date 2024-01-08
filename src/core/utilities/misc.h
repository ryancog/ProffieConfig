// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <initializer_list>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

class Misc
{
public:

# ifdef __WXOSX__
  static char path[PATH_MAX];
# endif

  struct numEntry;
  struct numEntryDouble;
  struct textEntry;
  class MessageBoxEvent;

  static wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;

  static numEntry createNumEntry(wxWindow* parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal);
  static numEntryDouble createNumEntryDouble(wxWindow* parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal);
  static textEntry createTextEntry(wxWindow* parent, wxString displayText, int32_t ID, wxString defaultOption, int32_t flags);

  static const wxArrayString createEntries(std::vector<wxString> list);
  static const wxArrayString createEntries(std::initializer_list<wxString> list);

private:
  Misc();
};

class Misc::MessageBoxEvent : public wxCommandEvent {
  public:
    MessageBoxEvent(int32_t id, wxString _message, wxString _caption, long _style = wxOK | wxCENTER){
      this->SetEventType(EVT_MSGBOX);
      this->SetId(id);
      caption = _caption;
      message = _message;
      style = _style;
    }

    wxString caption;
    wxString message;
    long style;
  };

struct Misc::numEntry {
  wxBoxSizer* box{nullptr};
  wxSpinCtrl* num{nullptr};
  wxStaticText* text{nullptr};
};

struct Misc::numEntryDouble {
  wxBoxSizer* box{nullptr};
  wxSpinCtrlDouble* num{nullptr};
  wxStaticText* text{nullptr};
};

struct Misc::textEntry {
  wxBoxSizer* box{nullptr};
  wxTextCtrl* entry{nullptr};
  wxStaticText* text{nullptr};
};
