#pragma once

#include <initializer_list>
#include <memory>
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

  enum {
    ID_WindowSelect,
    ID_Initialize,
    ID_RefreshDev,
    ID_ApplyChanges,
    ID_DeviceSelect,
    ID_Docs,
    ID_Issue,

    ID_GenFile,
    ID_VerifyConfig,
    ID_ExportFile,
    ID_ImportFile,

    ID_SerialCommand,
    ID_OpenSerial,
    ID_StyleEditor
  };

  struct numEntry {
    wxBoxSizer *box{nullptr};
    wxSpinCtrl *num{nullptr};
    wxStaticText* text{nullptr};
  };

  struct numEntryDouble {
    wxBoxSizer* box{nullptr};
    wxSpinCtrlDouble* num{nullptr};
    wxStaticText* text{nullptr};
  };

  struct comboBoxEntry {
    wxBoxSizer* box{nullptr};
    wxComboBox* entry{nullptr};
    wxStaticText* text{nullptr};
  };

  struct textEntry {
    wxBoxSizer* box{nullptr};
    wxTextCtrl* entry{nullptr};
    wxStaticText* text{nullptr};
  };

  class MessageBoxEvent : public wxCommandEvent {
  public:
    MessageBoxEvent(wxEventTypeTag<wxCommandEvent> tag, int32_t id, wxString _message, wxString _caption, long _style = wxOK | wxCENTER){
      this->SetEventType(tag);
      this->SetId(id);
      caption = _caption;
      message = _message;
      style = _style;
    }

    wxString caption;
    wxString message;
    long style;
  };

  static wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;

  static numEntry* createNumEntry(wxWindow* parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal);
  static numEntryDouble* createNumEntryDouble(wxWindow* parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal);
  static comboBoxEntry* createComboBoxEntry(wxWindow* parent, wxString displayText, int32_t ID, wxString defaultOption, wxArrayString options, int32_t flags);
  static textEntry* createTextEntry(wxWindow* parent, wxString displayText, int32_t ID, wxString defaultOption, int32_t flags);

  static const wxArrayString createEntries(std::vector<wxString> list);
  static const wxArrayString createEntries(std::initializer_list<wxString> list);

private:
  Misc();
};
