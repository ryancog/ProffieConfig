#include <wx/app.h>
#include "mainwindow.h"
#include "configuration.h"

class ProffieConfig : public wxApp {
public:
  virtual bool OnInit() {
    new MainWindow();
    Configuration::instance->readConfig();
    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
