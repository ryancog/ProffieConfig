#include "onboard.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include "../resources/icons/icon.xpm"

Onboard::Onboard() : wxWizard(nullptr, wxID_ANY, "Welcome to ProffieConfig", wxBitmap(icon_xpm), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE) {
  auto firstPage = new WelcomePage(this);
  RunWizard(firstPage);
}

Onboard::WelcomePage::WelcomePage(wxWizard* parent) : wxWizardPageSimple(parent) {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  SetSizerAndFit(sizer);
}
