#pragma once

#include <wx/wizard.h>

class Onboard : public wxWizard {
public:
  Onboard();

private:
  class WelcomePage;
};

class Onboard::WelcomePage : public wxWizardPageSimple {
public:
  WelcomePage(wxWizard*);
};
