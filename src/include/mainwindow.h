#include <wx/wx.h>

#include "generalpage.h"
#include "presetspage.h"
#include "bladespage.h"
#include "hardwarepage.h"

#pragma once

class MainWindow : public wxFrame {
public:
    MainWindow();

private:
    wxBoxSizer* master;
    wxComboBox* windowSelect;

    GeneralPage* general;
    PresetsPage* presets;
    BladesPage* blades;
    HardwarePage* hardware;

    void BindEvents();
    void setConfigDefaults();
    void CreateMenuBar();
    void CreatePages();

    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnButton(wxCommandEvent& event);
};
