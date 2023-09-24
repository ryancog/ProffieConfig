#include <wx/app.h>
#include "initializer.h"
#include "mainwindow.h"

class ProffieConfig : public wxApp {
public:
    virtual bool OnInit() {
        //Initializer* init = new Initializer();
        //init->Show(false);

        MainWindow *frame = new MainWindow();
        frame->Show(true);

        Initializer::setup();
        return true;
    }
};

wxIMPLEMENT_APP(ProffieConfig);

#ifdef __WXMSW__
#pragma message "Hello"
#endif

#ifdef __WXGTK__
#pragma message "Hello"
#endif
