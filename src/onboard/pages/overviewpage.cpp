#include "onboard/onboard.h"

#include <wx/sizer.h>

Onboard::Overview::Overview(wxWizard* parent) : wxWizardPageSimple(parent) {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  SetSizerAndFit(sizer);
}
