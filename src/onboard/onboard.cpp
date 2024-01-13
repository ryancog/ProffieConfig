// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "onboard/onboard.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/timer.h>
#include <wx/statline.h>
#include <wx/event.h>
#include "../resources/icons/icon.xpm"

#include "tools/arduino.h"
#include "core/utilities/misc.h"
#include "core/appstate.h"

Onboard* Onboard::instance{nullptr};
Onboard::Onboard() : wxFrame(nullptr, wxID_ANY, "ProffieConfig First-Time Setup", wxDefaultPosition, wxDefaultSize, wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX) {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto contentSizer = new wxBoxSizer(wxHORIZONTAL);
  auto icon = new wxStaticBitmap(this, wxID_ANY, wxBitmap(icon_xpm));
  contentSizer->Add(icon, wxSizerFlags(0).Border(wxRIGHT, 10));
  welcomePage = new Welcome(this);
  dependencyPage = new DependencyInstall(this);
  dependencyPage->Hide();
  overviewPage = new Overview(this);
  overviewPage->Hide();
  contentSizer->Add(welcomePage, wxSizerFlags(1).Expand());
  contentSizer->Add(dependencyPage, wxSizerFlags(1).Expand());
  contentSizer->Add(overviewPage, wxSizerFlags(1).Expand());

  auto buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  skip = new wxButton(this, ID_Skip, "Skip Introduction");
  skip->Hide();
  next = new wxButton(this, ID_Next, "Next >");
  cancel = new wxButton(this, ID_Cancel, "Cancel");
  buttonSizer->Add(skip, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10));
  buttonSizer->AddStretchSpacer();
  buttonSizer->Add(next, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));
  buttonSizer->Add(cancel, wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10));

  sizer->Add(contentSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));
  sizer->Add(new wxStaticLine(this, wxID_ANY), wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 10));
  sizer->Add(buttonSizer, wxSizerFlags(0).Expand());

  sizer->SetMinSize(wxSize(900, 430));
  SetSizerAndFit(sizer);

  bindEvents(); // Do this first so children can bind their events to parent state.

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  CentreOnScreen();
  Show(true);
}
Onboard::~Onboard() {
  instance = nullptr;
}

void Onboard::bindEvents() {
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    if (event.CanVeto() && wxMessageBox("Are you sure you want to cancel setup?", "Exit ProffieConfig", wxYES_NO | wxNO_DEFAULT | wxCENTER, this) == wxNO) {
      event.Veto();
      return;
    }
    event.Skip();
    if (!AppState::instance->firstRun) MainMenu::instance = new MainMenu();
  });
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Close(); }, ID_Cancel);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (wxMessageBox("Are you sure you want to skip the Introduction?\n"
                         "\n"
                         "The introduction covers all the basics and usage of ProffieConfig.\n"
                         "You can return to it any time via \"File\"->\"Re-Run First-Time Setup\".",

                         "Skip Introduction", wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION) == wxYES) {
          wxPostEvent(GetEventHandler(), wxCommandEvent(wxEVT_BUTTON, ID_Next));
        }
      }, ID_Skip);
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        if (welcomePage->IsShown()) {
          welcomePage->Hide();
          dependencyPage->Show();
        } else if (dependencyPage->IsShown()) {
          if (!dependencyPage->completedInstall) dependencyInstall(event);
          else {
            dependencyPage->Hide();
            overviewPage->Show();
            overviewPage->prepare();
          }
        } else if (overviewPage->IsShown()) {
          AppState::instance->firstRun = false;
          AppState::instance->saveState();
          Close(true);
        }
        update();
      }, ID_Next);
}

void Onboard::update() {
  if (overviewPage->IsShown()) {
    skip->Show();
    next->Enable(overviewPage->isDone);
    next->SetLabel("Finish");
  }
  else {
    skip->Hide();
    next->Enable();
    next->SetLabel("Next >");
  }

  Layout();
  Fit();
}

void Onboard::dependencyInstall(wxCommandEvent&) {
  Disable();
  dependencyPage->pressNext->Hide();
  dependencyPage->loadingBar->Show();
  dependencyPage->Layout();

  dependencyPage->barPulser->Start(50);

  Arduino::init(this, [=](bool succeeded) {
    Enable();
    dependencyPage->loadingBar->Hide();
    dependencyPage->barPulser->Stop();

    if (succeeded) {
      dependencyPage->description->Hide();
      dependencyPage->doneMessage->Show();
      dependencyPage->Layout();

      dependencyPage->completedInstall = true;
    } else {
      dependencyPage->pressNext->Show();
      dependencyPage->Layout();

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
