#pragma once

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>

#include "defines.h"
#include "misc.h"

class GeneralPage : public wxStaticBoxSizer
{
public:
    GeneralPage(wxWindow*);

    struct {
        wxComboBox *board{nullptr};
        wxCheckBox *massStorage{nullptr};
        wxCheckBox *webUSB{nullptr};

        wxComboBox *prop{nullptr};
        wxCheckBox *noLockupHold{nullptr};
        wxCheckBox *stabOn{nullptr};
        wxCheckBox *swingOn{nullptr};
        wxCheckBox *twistOn{nullptr};
        wxCheckBox *thrustOn{nullptr};
        wxCheckBox *twistOff{nullptr};

        //wxCheckBox *battleEnable{nullptr};
        wxCheckBox *gestureEnBattle{nullptr};
        Misc::numEntry lockupDelay;
        wxCheckBox *forcePush{nullptr};
        Misc::numEntry forcePushLength;

        Misc::numEntry buttons;
        Misc::numEntry volume;
        Misc::numEntry clash;
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
    GeneralPage();
};
