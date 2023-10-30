#include <wx/app.h>
#include "mainwindow.h"
#include "configuration.h"

class ProffieConfig : public wxApp {
public:
  virtual bool OnInit() {
    MainWindow *frame = new MainWindow();
    Configuration::instance->readConfig();
    frame->Show(true);

    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
