#pragma once

#include <cstdio>

#include <wx/progdlg.h>
#include <wx/sizer.h>

#include "appstate.h"

class Initializer : public wxProgressDialog {
public:
    Initializer();

    static void setup();

    static std::string message;
    static uint8_t progress;

private:
};
