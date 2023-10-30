#pragma once
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>

#include "misc.h"


class GeneralPage : public wxStaticBoxSizer
{
public:
  GeneralPage(wxWindow*);
  static GeneralPage* instance;

  struct {
    wxComboBox* board{nullptr};
    wxCheckBox* massStorage{nullptr};
    wxCheckBox* webUSB{nullptr};

    Misc::numEntry* buttons;
    Misc::numEntry* volume;
    Misc::numEntryDouble* clash;
    Misc::numEntry* pliTime;
    Misc::numEntry* idleTime;
    Misc::numEntry* motionTime;

    Misc::numEntry* maxLEDs;

    wxCheckBox* volumeSave{nullptr};
    wxCheckBox* presetSave{nullptr};
    wxCheckBox* colorSave{nullptr};
    wxCheckBox* disableColor{nullptr};
    wxCheckBox* disableDev{nullptr};
  } settings;

private:
  wxStaticBoxSizer* boardSettings(wxStaticBoxSizer*);
  wxStaticBoxSizer* optionSettings(wxStaticBoxSizer*);
  wxBoxSizer* boolOptions(wxStaticBoxSizer*);
  wxBoxSizer* numOptions(wxStaticBoxSizer*);

  GeneralPage();
};
