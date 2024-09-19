// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "onboard/onboard.h"

#include <wx/sizer.h>

Onboard::DependencyInstall::DependencyInstall(wxWindow* parent) : wxWindow(parent, ID_DependencyInstall) {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  barPulser = new wxTimer(this);
  Bind(wxEVT_TIMER, [&](wxTimerEvent&) { loadingBar->Pulse(); });

  auto title = createHeader(this, "Dependency Installation");
  description = new wxStaticText(this, wxID_ANY,
                                 "In order to continue, ProffieConfig needs to do some setup.\n"
                                 "This will involve the following:\n"
                                 "\n"
                                 "\t-Proffieboard Arduino Core Installation\n"
#                                     ifdef __WINDOWS__
                                 "\t-Proffieboard Driver Installation\n"
#                                     endif
                                 "\n\n"
                                 "An internet connection is required, and installation may take several minutes.\n"
#                                     ifdef __WINDOWS__
                                 "When the driver installation starts, you will be prompted, please follow the instructions in the new window.\n"
#                                     endif
                                 );
  pressNext = new wxStaticText(this, wxID_ANY, "Press \"Next\" to begin installation.\n");
  doneMessage = new wxStaticText(this, wxID_ANY, "The installation completed successfully. Press \"Next\" to continue...");
  doneMessage->Hide();

  loadingBar = new wxGauge(this, wxID_ANY, 50, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH);
  loadingBar->Hide();

  sizer->Add(title);
  sizer->AddSpacer(40);
  sizer->Add(description);
  sizer->Add(pressNext);
  sizer->Add(loadingBar);
  sizer->Add(doneMessage);
  SetSizerAndFit(sizer);
}
