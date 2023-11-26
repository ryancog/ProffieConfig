#pragma once

#include <initializer_list>
#include <memory>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

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

    ID_PropSelect,
    ID_PropOption,

    ID_PresetArrayList,
    ID_PresetList,
    ID_BladeList,
    ID_AddPresetArray,
    ID_RemovePresetArray,
    ID_AddPreset,
    ID_RemovePreset,
    ID_PresetChange,

    ID_BladeArray,

    ID_BladeSelect,
    ID_SubBladeSelect,
    ID_BladePower,
    ID_BladeOption,
    ID_AddBlade,
    ID_RemoveBlade,
    ID_AddSubBlade,
    ID_RemoveSubBlade,
    ID_BladeType,

    ID_BladeIDEnable,
    ID_BladeDetectEnable,
    ID_BladeIDMode,
    ID_BladeIDEnablePower,
    ID_BladeIDList,
    ID_BladeIDAdd,
    ID_BladeIDRemove,

    ID_SerialCommand,
    ID_OpenSerial,
    ID_StyleEditor
  };

  struct numEntry {
    wxBoxSizer *box{nullptr};
    wxSpinCtrl *num{nullptr};
  };

  struct numEntryDouble {
    wxBoxSizer *box{nullptr};
    wxSpinCtrlDouble *num{nullptr};
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

  static numEntry* createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal);
  static numEntryDouble* createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal);

  static const wxArrayString createEntries(std::vector<wxString> list);
  static const wxArrayString createEntries(std::initializer_list<wxString> list);

private:
  Misc();
};
