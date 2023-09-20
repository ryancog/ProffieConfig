#pragma once

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

#include "misc.h"
#include "configuration.h"

class BladesPage : public wxStaticBoxSizer
{
public:
    BladesPage(wxWindow*);

    static void update();

    struct {
        wxListBox *bladeSelect{nullptr};
        wxListBox *subBladeSelect{nullptr};
        wxButton *addBlade{nullptr};
        wxButton *removeBlade{nullptr};
        wxButton *addSubBlade{nullptr};
        wxButton *removeSubBlade{nullptr};

        wxComboBox *bladeType{nullptr};
        wxStaticText *bladeDataPinLabel{nullptr};
        wxComboBox *bladeDataPin{nullptr};
        wxStaticText *bladePixelsLabel{nullptr};
        wxSpinCtrl *bladePixels{nullptr};

        wxCheckBox *usePowerPin1{nullptr};
        wxCheckBox *usePowerPin2{nullptr};
        wxCheckBox *usePowerPin3{nullptr};
        wxCheckBox *usePowerPin4{nullptr};
        wxCheckBox *usePowerPin5{nullptr};
        wxCheckBox *usePowerPin6{nullptr};

        wxStaticText *bladeColorOrderLabel{nullptr};
        wxComboBox *blade3ColorOrder{nullptr};
        wxComboBox *blade4ColorOrder{nullptr};
        wxCheckBox *blade4UseRGB{nullptr};
        wxStaticText *star1ColorLabel{nullptr};
        wxComboBox *star1Color{nullptr};
        wxStaticText *star2ColorLabel{nullptr};
        wxComboBox *star2Color{nullptr};
        wxStaticText *star3ColorLabel{nullptr};
        wxComboBox *star3Color{nullptr};
        wxStaticText *star4ColorLabel{nullptr};
        wxComboBox *star4Color{nullptr};

        wxCheckBox *subBladeUseStride{nullptr};
        wxStaticText *subBladeStartLabel{nullptr};
        wxSpinCtrl *subBladeStart{nullptr};
        wxStaticText *subBladeEndLabel{nullptr};
        wxSpinCtrl *subBladeEnd{nullptr};
    } static settings;

    static int lastBladeSelection;
    static int lastSubBladeSelection;
private:
    BladesPage();
};
