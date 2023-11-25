#include <wx/app.h>
#include "mainwindow.h"
#include "configuration.h"

#ifdef __WXOSX__
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

class ProffieConfig : public wxApp {
public:
  virtual bool OnInit() {

#   ifdef __WXOSX__
    char path[PATH_MAX];
    uint32_t pathLen = sizeof(path);
    _NSGetExecutablePath(path, &pathLen);
    chdir(dirname(path));
    chdir("../../");
#   endif

    MainWindow::instance = new MainWindow();
    Configuration::instance->readConfig();
    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
