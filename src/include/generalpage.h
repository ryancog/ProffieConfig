#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>

#include "defines.h"
#include "misc.h"

#pragma once

class GeneralPage : public wxStaticBoxSizer
{
public:
  GeneralPage(wxWindow*);

  struct {
    wxComboBox *board{nullptr};
    wxCheckBox *massStorage{nullptr};
    wxCheckBox *webUSB{nullptr};

    Misc::numEntry buttons;
    Misc::numEntry volume;
    Misc::numEntryDouble clash;
    Misc::numEntry pliTime;
    Misc::numEntry idleTime;
    Misc::numEntry motion;
    wxCheckBox *volumeSave{nullptr};
    wxCheckBox *presetSave{nullptr};
    wxCheckBox *colorSave{nullptr};
    wxCheckBox *disableColor{nullptr};
    wxCheckBox *disableDev{nullptr};
  } static settings;



private:
  void createBoardSettings();
  void createOptionSettings();

  GeneralPage();


  wxBoxSizer *generalHoriz{nullptr};
  wxStaticBoxSizer *boardSetup{nullptr};
  wxStaticBoxSizer *options{nullptr};
};
