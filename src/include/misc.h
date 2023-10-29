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
  enum {
    ID_WindowSelect,
    ID_Initialize,
    ID_RefreshDevButton,
    ID_ApplyButton,
    ID_DevSelect,
    ID_PropSelect,
    ID_PropOption,
    ID_GenFile,
    ID_ExportFile,
    ID_ImportFile,
    ID_PresetList,
    ID_BladeList,
    ID_PresetEditor,
    ID_AddPreset,
    ID_RemovePreset,
    ID_PresetName,
    ID_PresetDir,
    ID_PresetTrack,
    ID_BladeSelect,
    ID_SubBladeSelect,
    ID_BladePower,
    ID_BladeOption,
    ID_AddBlade,
    ID_RemoveBlade,
    ID_AddSubBlade,
    ID_RemoveSubBlade,
    ID_BladeType
  };

  struct numEntry {
    wxBoxSizer *box{nullptr};
    wxSpinCtrl *num{nullptr};
  };

  struct numEntryDouble {
    wxBoxSizer *box{nullptr};
    wxSpinCtrlDouble *num{nullptr};
  };

  static numEntry* createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal);
  static numEntryDouble* createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal);

  static const wxArrayString createEntries(std::vector<std::string> list);
  static const wxArrayString createEntries(std::initializer_list<std::string> list);

private:
  Misc();
};
