#include <cstdio>

#include <wx/progdlg.h>
#include <wx/sizer.h>

#pragma once

class Initializer : public wxProgressDialog {
public:
    Initializer();

    static void setup();

    static std::string message;
    static uint8_t progress;

private:
};
