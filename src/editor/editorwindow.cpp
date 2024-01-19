// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/editorwindow.h"

#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/propspage.h"
#include "editor/dialogs/bladearraydlg.h"

#include "core/config/settings.h"
#include "core/config/configuration.h"
#include "core/defines.h"
#include "core/utilities/misc.h"
#include "core/utilities/progress.h"

#include "tools/arduino.h"

#include <wx/event.h>
#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>

EditorWindow::EditorWindow(const std::string& _configName, wxWindow* parent) : wxFrame(parent, wxID_ANY, "ProffieConfig Editor - " + _configName, wxDefaultPosition, wxDefaultSize), openConfig(_configName) {
  createMenuBar();
  createPages();
  bindEvents();
  createToolTips();
  settings = new Settings(this);

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif
  sizer->SetMinSize(450, -1);
}

void EditorWindow::bindEvents() {
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) {
    if (!event.CanVeto()) {
      event.Skip();
      return;
    }
    if (wxMessageBox("Are you sure you want to close the editor?\n\nAny unsaved changes will be lost!", "Close ProffieConfig Editor", wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, this) == wxYES) {
      Hide();
    }
    event.Veto();
  });
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::outputConfig(CONFIG_DIR + openConfig + ".h", this); }, ID_SaveConfig);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(this); }, ID_ExportConfig);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(this, this); }, ID_VerifyConfig);

# if defined(__WXOSX__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(Misc::path + std::string("/" STYLEEDIT_PATH)); }, ID_StyleEditor);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(STYLEEDIT_PATH); }, ID_StyleEditor);
#endif

  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        generalPage->Show(windowSelect->entry()->GetValue() == "General");
        propsPage->Show(windowSelect->entry()->GetValue() == "Prop File");
        bladesPage->Show(windowSelect->entry()->GetValue() == "Blade Arrays");
        presetsPage->Show(windowSelect->entry()->GetValue() == "Presets And Styles");

        //generalPage->update();
        bladesPage->update();
        propsPage->update();
        presetsPage->update();

        FULLUPDATEWINDOW(this);
        if (propsPage->IsShown()) {
          propsPage->SetMinClientSize(wxSize(propsPage->sizer->GetMinSize().GetWidth(), 0));
          sizer->Layout();
          SetSizerAndFit(sizer);
          SetSize(wxSize(GetSize().GetWidth(), GetMinHeight() + propsPage->GetBestVirtualSize().GetHeight()));
          SetMinSize(wxSize(GetSize().GetWidth(), 350));
        }
      }, ID_WindowSelect);
}
void EditorWindow::createToolTips() {
}

void EditorWindow::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_VerifyConfig, "Verify Config\tCtrl+R");
  file->AppendSeparator();
  file->Append(ID_SaveConfig, "Save Config\tCtrl+S");
  file->Append(ID_ExportConfig, "Export Config...\t");

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
  windowSelect = new pcComboBox(this, ID_WindowSelect, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles"}), wxCB_READONLY | wxCB_DROPDOWN);
  options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));

  generalPage = new GeneralPage(this);
  propsPage = new PropsPage(this);
  presetsPage = new PresetsPage(this);
  bladesPage = new BladesPage(this);

  //generalPage->update();
  propsPage->update();
  presetsPage->update();
  bladesPage->update();

  propsPage->Show(false);
  bladesPage->Show(false);
  presetsPage->Show(false);

  sizer->Add(options, wxSizerFlags(0).Expand());
  sizer->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(propsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  sizer->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(sizer);
}


const std::string& EditorWindow::getOpenConfig() { return openConfig; }
