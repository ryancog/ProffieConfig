// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include <wx/app.h>
#include "core/mainwindow.h"
#include "config/configuration.h"
#include "../resources/icons/icon.xpm"
#include <wx/splash.h>

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

    wxIcon icon{wxICON(icon)};
    MainWindow::instance = new MainWindow();

    wxSplashScreen(wxBitmap(icon), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, MainWindow::instance, wxID_ANY);
    Configuration::instance->readConfig();
    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
