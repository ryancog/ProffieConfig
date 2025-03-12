// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "app/app.h"
#include "core/appstate.h"

#include <wx/app.h>

class ProffieConfig : public wxApp {
public:
    virtual bool OnInit() override {
        App::init("ProffieConfig");
        wxImage::AddHandler(new wxPNGHandler());
        AppState::init();

        return true;
    }

    virtual void OnUnhandledException() override {
        App::exceptionHandler();
    }
};

wxIMPLEMENT_APP(ProffieConfig);
