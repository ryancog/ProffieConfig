#pragma once

#include "misc.h"
#include "configuration.h"

class HardwarePage : public wxStaticBoxSizer
{
public:
    HardwarePage(wxWindow*);

    static void update();

    struct {
        wxCheckBox *bladeDetect{nullptr};
        wxStaticText *bladeDetectPinLabel{nullptr};
        wxComboBox *bladeDetectPin{nullptr};
    } static settings;
private:
    HardwarePage();
};
