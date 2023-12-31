#include "onboard/onboard.h"

#include <wx/sizer.h>

Onboard::Welcome::Welcome(wxWindow* parent) : wxWindow(parent, ID_Welcome) {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto welcomeText = createHeader(this, "Welcome to ProffieConfig " VERSION "!");
  auto createByText = new wxStaticText(this, wxID_ANY, "Created by Ryryog25");

  auto infoText = new wxStaticText(this, wxID_ANY,
                                   "ProffieConfig is an All-in-One utility for managing your Proffieboard.\n"
                                   "Links to documentation can be found in the application under Help->Documentation...\n"
                                   "\n"
                                   "This wizard will guide you through first-time setup and usage of ProffieConfig.\n"
                                   "\n\n"
                                   "Press \"Next\" when you're ready to continue, and we'll get started!"
                                   , wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);

  sizer->Add(welcomeText, wxSizerFlags(0).Center());
  sizer->Add(createByText, wxSizerFlags(0).Center());
  sizer->AddSpacer(40);
  sizer->Add(infoText, wxSizerFlags(0).Center());
  SetSizerAndFit(sizer);
}
