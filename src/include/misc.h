#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>

#pragma once

class Misc
{
public:
    enum {
        ID_WindowSelect,
        ID_GenFile,
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
        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSubBlade,
        ID_RemoveSubBlade,
        ID_BladeType,
    };

    struct numEntry {
        wxBoxSizer *box{nullptr};
        wxSpinCtrl *num{nullptr};
        wxSpinCtrlDouble *doubleNum{nullptr};
    };

    static numEntry createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal);
    static numEntry createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal);

private:
    Misc();
};
