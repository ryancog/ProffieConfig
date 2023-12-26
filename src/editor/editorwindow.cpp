// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "editor/editorwindow.h"

#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/propspage.h"
#include "editor/pages/bladearraypage.h"

#include "core/config/settings.h"
#include "core/config/configuration.h"
#include "core/defines.h"
#include "core/utilities/misc.h"
#include "core/utilities/progress.h"
#include "core/appstate.h"

#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>

EditorWindow::EditorWindow(const std::string& config) : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  createMenuBar();
  createPages();
  bindEvents();
  createToolTips();
  settings = new Settings(this);

  if (!Configuration::readConfig(config, this)) {
    Destroy();
    return;
  }
# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  Show(true);
}


void EditorWindow::bindEvents() {
  // Main Window
  // Yeah, this segfaults right now... but we want it to close anyways, right? I need to fix this... I have a few ideas I'll try when I get back to it.
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) { if (wxMessageBox("Are you sure you want to close ProffieConfig?\n\nAny unsaved changes will be lost!", "Close ProffieConfig", wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxNO && event.CanVeto()) event.Veto(); else { AppState::instance->saveState(); Destroy(); }});
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);

# if defined(__WXOSX__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(Misc::path + std::string("/" STYLEEDIT_PATH)); }, ID_StyleEditor);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(STYLEEDIT_PATH); }, ID_StyleEditor);
#endif

   Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        generalPage->Show(windowSelect->GetValue() == "General");
        propPage->Show(windowSelect->GetValue() == "Prop File");
        bladesPage->Show(windowSelect->GetValue() == "Blade Arrays");
        presetsPage->Show(windowSelect->GetValue() == "Presets And Styles");
        idPage->Show(windowSelect->GetValue() == "Blade Awareness");

        //generalPage->update();
        idPage->update();
        propPage->update();
        bladesPage->update();
        presetsPage->update();

        FULLUPDATEWINDOW(this);
        if (propPage->IsShown()) {
          propPage->SetMinClientSize(wxSize(propPage->sizer->GetMinSize().GetWidth(), 0));
          sizer->Layout();
          SetSizerAndFit(sizer);
          SetSize(wxSize(GetSize().GetWidth(), GetMinHeight() + propPage->GetBestVirtualSize().GetHeight()));
          SetMinSize(wxSize(GetSize().GetWidth(), 350));
        }
      }, ID_WindowSelect);
}
void EditorWindow::createToolTips() {
}

void EditorWindow::createMenuBar() {
  wxMenu *file = new wxMenu;
   file->Append(wxID_ABOUT);
   file->Append(wxID_EXIT);

  wxMenu* tools = new wxMenu;
  tools->Append(ID_StyleEditor, "Style Editor...", "Open the ProffieOS style editor");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  menuBar->Append(tools, "&Tools");
  SetMenuBar(menuBar);
}

void EditorWindow::createPages() {
  sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
  windowSelect = new wxComboBox(this, ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles", "Blade Awareness"  /*, "Hardware"*/}), wxCB_READONLY | wxCB_DROPDOWN);
  options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));

  generalPage = new GeneralPage(this);
  propPage = new PropsPage(this);
  presetsPage = new PresetsPage(this);
  bladesPage = new BladesPage(this);
  idPage = new BladeArrayPage(this);

  //generalPage->update();
  propPage->update();
  presetsPage->update();
  bladesPage->update();
  idPage->update();

  propPage->Show(false);
  bladesPage->Show(false);
  presetsPage->Show(false);
  idPage->Show(false);

  sizer->Add(options, wxSizerFlags(0).Expand());
  sizer->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(propPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(idPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(sizer); // use the sizer for layout and set size and hints
}
