#include <wx/app.h>
#include "mainwindow.h"

class ProffieConfig : public wxApp {
public:
  virtual bool OnInit() {
    MainWindow *frame = new MainWindow();
    frame->Show(true);

    return true;
  }
};

wxIMPLEMENT_APP(ProffieConfig);
