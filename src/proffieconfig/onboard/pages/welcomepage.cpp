#include "../onboard.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/sizer.h>

Onboard::Welcome::Welcome(wxWindow* parent) : wxPanel(parent, OnboardFrame::ID_Welcome) {
  auto *sizer{new wxBoxSizer(wxVERTICAL)};

  auto *welcomeText{OnboardFrame::createHeader(this, wxString::Format(_("Welcome to ProffieConfig %s!"), wxSTRINGIZE(EXEC_VERSION)))};

  auto *infoText{new wxStaticText(this, wxID_ANY, _(
              "ProffieConfig is an All-in-One utility for managing your Proffieboard.\n"
              "Links to documentation can be found in the application under Help->Documentation...\n"
              "\n"
              "This wizard will guide you through first-time setup and usage of ProffieConfig.\n"
              "\n\n"
              "Press \"Next\" when you're ready to continue, and we'll get started!"))
  };

  sizer->Add(welcomeText, wxSizerFlags(0).Center());
  sizer->AddSpacer(40);
  sizer->Add(infoText, wxSizerFlags(0).Center());
  SetSizerAndFit(sizer);
}
