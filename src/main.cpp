#include <wx/app.h>
#include "initializer.h"
#include "mainwindow.h"

class ProffieConfig : public wxApp {
public:
    virtual bool OnInit() {
        Initializer* init = new Initializer();
        init->Show(true);

        MainWindow *frame = new MainWindow();
        frame->Show(false);

        Initializer::setup();
        return true;
    }
};

wxIMPLEMENT_APP(ProffieConfig);
