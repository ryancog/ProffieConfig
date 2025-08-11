// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/app.h>
#include <wx/image.h>

#include "app/app.h"
#include "config/info.h"
#include "core/appstate.h"
#include "ui/message.h"

class ProffieConfig : public wxApp {
public:
    bool OnInit() override {
        if (not App::init("ProffieConfig")) {
            PCUI::showMessage(_("ProffieConfig is Already Running"), App::getAppName());
            return false;
        }

        Config::setExecutableVersion(wxSTRINGIZE(VERSION));

        AppState::init();

        return true;
    }

    bool OnExceptionInMainLoop() override {
        App::exceptionHandler();
        return false;
    }

    void OnUnhandledException() override {
        App::exceptionHandler();
    }
};

wxIMPLEMENT_APP(ProffieConfig);
