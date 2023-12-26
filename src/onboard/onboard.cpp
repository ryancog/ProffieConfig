// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "onboard.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/timer.h>
#include "../resources/icons/icon.xpm"

#include "tools/arduino.h"
#include "core/utilities/misc.h"

Onboard::Onboard() : wxWizard(nullptr, wxID_ANY, "ProffieConfig First-Time Setup", wxBitmap(icon_xpm), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE) {
  SetPageSize(wxSize(600, -1));

  bindEvents();
}

bool Onboard::run() {
  auto firstPage = new Welcome(this);
  (*firstPage)
      .Chain(new DependencyInstall(this))
      .Chain(new Overview(this));

  return RunWizard(firstPage);
}

void Onboard::bindEvents() {
  Bind(wxEVT_WIZARD_CANCEL, [&](wxWizardEvent& event) { if (wxMessageBox("Are you sure you want to cancel setup?", "Exit ProffieConfig", wxYES_NO | wxNO_DEFAULT | wxCENTER, this) == wxNO) event.Veto(); });
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_WIZARD_BEFORE_PAGE_CHANGED, [&](wxWizardEvent& event) {
    if (event.GetPage()->GetId() == ID_DependencyInstall && event.GetDirection()) {
      auto page = static_cast<DependencyInstall*>(event.GetPage());

      if (!page->completedInstall) event.Veto();
      else return;

      Disable();
      page->pressNext->Hide();
      page->loadingBar->Show();
      page->Layout();

      page->barPulser->Start(50);

      Arduino::init(this, [=](bool succeeded) {
        Enable();
        page->loadingBar->Hide();
        page->barPulser->Stop();

        if (succeeded) {
          page->description->Hide();
          page->doneMessage->Show();
          page->Layout();

          page->completedInstall = true;
        } else {
          page->pressNext->Show();
          page->Layout();

          wxMessageBox("Dependency installation failed, please try again.", "Installation Failure", wxOK | wxCENTER, this);
        }
      });
    }
  });
}

Onboard::Welcome::Welcome(wxWizard* parent) : wxWizardPageSimple(parent) {
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
  SetId(ID_Welcome);
}
Onboard::DependencyInstall::DependencyInstall(wxWizard* parent) : wxWizardPageSimple(parent) {
  auto sizer = new wxBoxSizer(wxVERTICAL);
  barPulser = new wxTimer(this);
  Bind(wxEVT_TIMER, [&](wxTimerEvent&) { loadingBar->Pulse(); });

  auto title = createHeader(this, "Dependency Installation");
  description = new wxStaticText(this, wxID_ANY,
                                      "In order to continue, ProffieConfig needs to do some setup.\n"
                                      "This will involve the following:\n"
                                      "\n"
                                      "\t-Proffieboard Arduino Core Installation\n"
#                                     ifdef __WXMSW__
                                      "\t-Proffieboard Driver Installation\n"
#                                     endif
                                      "\n\n"
                                      "An internet connection is required, and installation may take several minutes.\n"
#                                     ifdef __WXMSW__
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
  SetId(ID_DependencyInstall);
}
Onboard::Overview::Overview(wxWizard* parent) : wxWizardPageSimple(parent) {

}

wxStaticText* Onboard::createHeader(wxWindow* parent, const wxString& text) {
  auto header = new wxStaticText(parent, wxID_ANY, text);
  auto font = header->GetFont();
  font.MakeBold();
  font.SetPointSize(20);
  header->SetFont(font);

  return header;
}
