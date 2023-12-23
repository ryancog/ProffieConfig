// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/defines.h"
#include "core/mainwindow.h"
#include "core/appstate.h"
#include "config/configuration.h"
#include "../resources/icons/icon.xpm"

#include <wx/splash.h>
#include <wx/app.h>

#ifdef __WXOSX__
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

class ProffieConfig : public wxApp {
public:
  virtual bool OnInit() {

#   ifdef __WXOSX__
    uint32_t pathLen = sizeof(Misc::path);
    _NSGetExecutablePath(Misc::path, &pathLen);
    strncpy(Misc::path, dirname(Misc::path), PATH_MAX);
    chdir(Misc::path);
#   endif

    wxBitmap iconbmp(icon_xpm);
    new wxSplashScreen(iconbmp, wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 6000, nullptr, wxID_ANY);

    AppState::init();
    MainWindow::instance = new MainWindow();

    wxSplashScreen(wxBitmap(icon), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, MainWindow::instance, wxID_ANY);
    Configuration::instance->readConfig();
    for (const std::string& prop : AppState::instance->getProps()) {
      AppState::instance->addProp(PropFile(PROPCONFIG_DIR + prop + ".pconf"));
    }

    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
