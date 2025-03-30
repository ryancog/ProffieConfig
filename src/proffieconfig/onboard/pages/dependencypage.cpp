#include "../onboard.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/sizer.h>

Onboard::DependencyInstall::DependencyInstall(wxWindow* parent) : wxPanel(parent, OnboardFrame::ID_DependencyInstall) {
  auto *sizer{new wxBoxSizer(wxVERTICAL)};
  barPulser = new wxTimer(this);
  Bind(wxEVT_TIMER, [&](wxTimerEvent&) { loadingBar->Pulse(); });

  auto *title{OnboardFrame::createHeader(this, _("Dependency Installation"))};

  auto bulletString{wxString::FromUTF8("\t• ")};
  description = new wxStaticText(this, wxID_ANY,
          _("In order to continue, ProffieConfig needs to do some setup.") + '\n' +
          _("This will involve the following:") +
          "\n\n" +
          bulletString + _("ProffieOS Download") + '\n' +
          bulletString + _("Proffieboard Arduino Core Installation") + '\n' +
#         ifdef __WINDOWS__
          bulletString + _("Proffieboard Driver Installation") + '\n' +
#         endif
          '\n' +
          _("An internet connection is required, and installation may take several minutes.")
#         ifdef __WINDOWS__
          + '\n' + _("When the driver installation starts, you will be prompted, please follow the instructions in the new window.")
#         endif
          );
  pressNext = new wxStaticText(this, wxID_ANY, _("Press \"Next\" to begin installation.\n"));
  doneMessage = new wxStaticText(this, wxID_ANY, _("The installation completed successfully. Press \"Next\" to continue..."));
  doneMessage->Hide();

  loadingBar = new wxGauge(this, wxID_ANY, 50, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH);
  loadingBar->Hide();

  sizer->Add(title);
  sizer->AddSpacer(40);
  sizer->Add(description);
  sizer->AddSpacer(10);
  sizer->Add(pressNext);
  sizer->Add(loadingBar);
  sizer->Add(doneMessage);
  SetSizerAndFit(sizer);
}
