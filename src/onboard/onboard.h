// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include <wx/wizard.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>

class Onboard : public wxWizard {
public:
  Onboard();

private:
  static wxStaticText* createHeader(wxWindow*, const wxString&);
  void bindEvents();

  void dependencyInstall(wxWizardEvent&);

  class Welcome;
  class DependencyInstall;
  class Overview;

  enum {
    ID_Welcome,
    ID_DependencyInstall,
  };
};

class Onboard::Welcome : public wxWizardPageSimple {
public:
  Welcome(wxWizard*);
};

class Onboard::DependencyInstall : public wxWizardPageSimple {
public:
  DependencyInstall(wxWizard*);

  wxStaticText* description{nullptr};
  wxStaticText* pressNext{nullptr};
  wxStaticText* doneMessage{nullptr};
  wxGauge* loadingBar{nullptr};
  wxTimer* barPulser{nullptr};
  bool completedInstall{false};
};

class Onboard::Overview : public wxWizardPageSimple {
public:
  Overview(wxWizard*);
};
