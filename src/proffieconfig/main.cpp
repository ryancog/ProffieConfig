// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/app.h>
#include <wx/image.h>

#include "app/app.h"
#include "core/appstate.h"
#include "ui/message.h"

class ProffieConfig : public wxApp {
public:
    bool OnInit() override {
        if (not App::init("ProffieConfig")) {
            PCUI::showMessage(_("ProffieConfig is Already Running"), App::getAppName());
            return false;
        }

        wxInitAllImageHandlers();
        AppState::init();

        // Add updates for styleDisplay
        // Blades add/remove/modify

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
