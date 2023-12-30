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
#include "core/appstate.h"
#include "wx/wizard.h"

Onboard::Onboard() : wxWizard(nullptr, wxID_ANY, "ProffieConfig First-Time Setup", wxBitmap(icon_xpm), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE) {
  SetPageSize(wxSize(600, -1));
  bindEvents();

  auto firstPage = new Welcome(this);
  (*firstPage)
      .Chain(new DependencyInstall(this))
      .Chain(new Overview(this));

  if (RunWizard(firstPage)) {
    AppState::instance->firstRun = false;
    AppState::instance->saveState();
    MainMenu::instance = new MainMenu();
  }
}


void Onboard::bindEvents() {
  Bind(wxEVT_WIZARD_CANCEL, [&](wxWizardEvent& event) {
    if (wxMessageBox("Are you sure you want to cancel setup?", "Exit ProffieConfig", wxYES_NO | wxNO_DEFAULT | wxCENTER, this) == wxNO) {
      event.Veto();
      return;
    }
    if (!AppState::instance->firstRun) MainMenu::instance = new MainMenu();
  });
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_WIZARD_BEFORE_PAGE_CHANGED, [&](wxWizardEvent& event) { if (event.GetPage()->GetId() == ID_DependencyInstall && event.GetDirection()) dependencyInstall(event); });
  //Bind(wxEVT_WIZARD_PAGE_CHANGED, )
}

void Onboard::dependencyInstall(wxWizardEvent& event) {
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

wxStaticText* Onboard::createHeader(wxWindow* parent, const wxString& text) {
  auto header = new wxStaticText(parent, wxID_ANY, text);
  auto font = header->GetFont();
  font.MakeBold();
  font.SetPointSize(20);
  header->SetFont(font);

  return header;
}
