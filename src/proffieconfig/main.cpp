// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "core/appstate.h"

#include <wx/app.h>

class ProffieConfig : public wxApp {
public:
    virtual bool OnInit() override {
#   	ifdef __WXMSW__
        MSWEnableDarkMode();
        if (AttachConsole(ATTACH_PARENT_PROCESS)){
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            freopen("CONIN$", "r", stdin);
        }
#   	endif

        argv[0].find_last_of("/\\");
        chdir(argv[0].substr(0, argv[0].find_last_of("/\\")).c_str());

        AppState::init();

        return true;
    }
};

wxIMPLEMENT_APP(ProffieConfig);
