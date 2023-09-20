#include <wx/app.h>
#include "mainwindow.h"
#include "initializer.h"

class ProffieConfig : public wxApp {
public:
    virtual bool OnInit() {

        //Initializer* init = new Initializer();
        //init->Show(true);

        MainWindow *frame = new MainWindow();
        frame->Show(true);

        //Initializer::setup();
        return true;
    }
};

wxIMPLEMENT_APP(ProffieConfig);
