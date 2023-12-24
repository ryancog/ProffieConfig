// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "editor/editorwindow.h"

#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/proppage.h"
#include "editor/pages/bladeidpage.h"

#include "core/config/configuration.h"
#include "core/config/settings.h"
#include "core/defines.h"
#include "tools/arduino.h"
#include "tools/serialmonitor.h"
#include "core/utilities/misc.h"
#include "core/appstate.h"

#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>

EditorWindow* EditorWindow::instance{nullptr};
EditorWindow::EditorWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  instance = this;
  createMenuBar();
  createPages();
  loadProps();
  bindEvents();
  createToolTips();
  settings = new Settings();

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  Show(true);
}


void EditorWindow::bindEvents() {
  // Main Window
  // Yeah, this segfaults right now... but we want it to close anyways, right? I need to fix this... I have a few ideas I'll try when I get back to it.
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) { if (wxMessageBox("Are you sure you want to close ProffieConfig?\n\nAny unsaved changes will be lost!", "Close ProffieConfig", wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, EditorWindow::instance) == wxNO && event.CanVeto()) event.Veto(); else { AppState::instance->saveState(); Destroy(); }});
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent(progDialog, (Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::init(); }, ID_Initialize);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(ABOUT_MESSAGE, "About ProffieConfig", wxOK | wxICON_INFORMATION, EditorWindow::instance); }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION, EditorWindow::instance); }, ID_Copyright);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::outputConfig(); AppState::instance->setSaved(); }, ID_GenFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(); }, ID_VerifyConfig);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(); }, ID_ExportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::importConfig(); }, ID_ImportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/blob/master/docs"); }, ID_Docs);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/Ryryog25/ProffieConfig/issues/new"); }, ID_Issue);

# if defined(__WXOSX__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(Misc::path + std::string("/" STYLEEDIT_PATH)); }, ID_StyleEditor);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(STYLEEDIT_PATH); }, ID_StyleEditor);
#endif

# if defined(__WXMSW__)
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { SerialMonitor::instance = new SerialMonitor; SerialMonitor::instance->Close(true); }, ID_OpenSerial);
# else
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise(); else SerialMonitor::instance = new SerialMonitor(); }, ID_OpenSerial);
#endif

  Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        if (devSelect->GetValue() == "Select Device...") applyButton->Disable();
        else applyButton->Enable();
        if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Close(true);
      }, ID_DeviceSelect);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(); }, ID_RefreshDev);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::applyToBoard(); }, ID_ApplyChanges);
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

        FULLUPDATEWINDOW;
        if (propPage->IsShown()) {
          propPage->SetMinClientSize(wxSize(propPage->sizer->GetMinSize().GetWidth(), 0));
          master->Layout();
          SetSizerAndFit(master);
          SetSize(wxSize(GetSize().GetWidth(), GetMinHeight() + propPage->GetBestVirtualSize().GetHeight()));
          SetMinSize(wxSize(GetSize().GetWidth(), 350));
        }
      }, ID_WindowSelect);
}
void EditorWindow::createToolTips() {
  TIP(applyButton, "Apply the current configuration to the selected Proffieboard.");
  TIP(devSelect, "Select the Proffieboard to connect to.\nThis will be an unrecognizable device identifier, but chances are there's only one which will show up.");
  TIP(refreshButton, "Refresh the detected boards.");
}

void EditorWindow::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_GenFile, "Save Config\tCtrl+S", "Generate Config File");
  file->Append(ID_ExportFile, "Export Config...", "Choose a location to save a copy of your config...");
  file->Append(ID_ImportFile, "Import Config...", "Choose a file to import...");
  file->AppendSeparator();
  file->Append(ID_VerifyConfig, "Verify Config\tCtrl+R", "Generate Config and Compile to test...");
  file->AppendSeparator();
  file->Append(ID_Initialize, "Install Dependencies...", "Install Platform-Specific Proffieboard Dependencies");
  file->AppendSeparator();
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, "Copyright Notice");
  file->Append(wxID_EXIT);

  wxMenu* board = new wxMenu;
  board->Append(ID_OpenSerial, "Serial Monitor...\tCtrl+M", "Open a serial monitor to the proffieboard");

  wxMenu* tools = new wxMenu;
  tools->Append(ID_StyleEditor, "Style Editor...", "Open the ProffieOS style editor");

  wxMenu* help = new wxMenu;
  help->Append(ID_Docs, "Documentation...\tCtrl+H", "Open the ProffieConfig docs in your web browser");
  help->Append(ID_Issue, "Help/Bug Report...", "Open GitHub to submit issue");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  menuBar->Append(board, "&Board");
  menuBar->Append(tools, "&Tools");
  menuBar->Append(help, "&Help");
  SetMenuBar(menuBar);
}

void EditorWindow::createPages() {
  master = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
  windowSelect = new wxComboBox(this, ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles", "Blade Awareness"  /*, "Hardware"*/}), wxCB_READONLY | wxCB_DROPDOWN);
  refreshButton = new wxButton(this, ID_RefreshDev, "Refresh", wxDefaultPosition, wxDefaultSize, 0);
  devSelect = new wxComboBox(this, ID_DeviceSelect, "Select Device...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  applyButton = new wxButton(this, ID_ApplyChanges, "Apply to Board", wxDefaultPosition, wxDefaultSize, 0);
  applyButton->Disable();
  options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->AddStretchSpacer(1);
  options->Add(refreshButton, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(devSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(applyButton, wxSizerFlags(0).Border(wxALL, 10));

  generalPage = new GeneralPage(this);
  propPage = new PropPage(this);
  presetsPage = new PresetsPage(this);
  bladesPage = new BladesPage(this);
  idPage = new BladeIDPage(this);

  //generalPage->update();
  propPage->update();
  presetsPage->update();
  bladesPage->update();
  idPage->update();

  propPage->Show(false);
  bladesPage->Show(false);
  presetsPage->Show(false);
  idPage->Show(false);

  master->Add(options, wxSizerFlags(0).Expand());
  master->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(propPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(idPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(master); // use the sizer for layout and set size and hints
}
void EditorWindow::loadProps() {
  AppState::instance->clearProps();
  for (const auto& prop : AppState::instance->getPropFileNames()) {
    auto propConfig = PropFile::createPropConfig(prop);
    if (propConfig != nullptr) AppState::instance->addProp(propConfig);
  }
  propPage->updateProps();
}
