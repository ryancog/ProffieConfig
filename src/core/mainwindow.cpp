// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/mainwindow.h"

#include "config/configuration.h"
#include "core/defines.h"
#include "tools/arduino.h"
#include "elements/misc.h"
#include "tools/serialmonitor.h"
#include "pages/bladespage.h"
#include "pages/generalpage.h"
#include "pages/presetspage.h"
#include "pages/proppage.h"
#include "pages/bladeidpage.h"

#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/wx.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>

MainWindow* MainWindow::instance;
MainWindow::MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
  instance = this;
  Configuration::instance = new Configuration();
  createMenuBar();
  createPages();
  bindEvents();
  createToolTips();

# ifdef __WXMSW__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  Show(true);
}


void MainWindow::bindEvents() {
  // Main Window
  // Yeah, this segfaults right now... but we want it to close anyways, right? I need to fix this... I have a few ideas I'll try when I get back to it.
  Bind(wxEVT_CLOSE_WINDOW, [](wxCloseEvent& event ) { if (wxMessageBox("Are you sure you want to close ProffieConfig?\n\nAny unsaved changes will be lost!", "Close ProffieConfig", wxICON_WARNING | wxYES_NO | wxNO_DEFAULT, MainWindow::instance) == wxNO && event.CanVeto()) event.Veto(); else MainWindow::instance->Destroy(); });
  Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent(progDialog, (Progress::ProgressEvent*)&event); }, wxID_ANY);
  Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) { wxMessageBox(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this); }, wxID_ANY);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::init(); }, ID_Initialize);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(ABOUT_MESSAGE, "About ProffieConfig", wxOK | wxICON_INFORMATION, MainWindow::instance); }, wxID_ABOUT);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxMessageBox(COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION, MainWindow::instance); }, ID_Copyright);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->outputConfig(); }, ID_GenFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(); }, ID_VerifyConfig);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->exportConfig(); }, ID_ExportFile);
  Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::instance->importConfig(); }, ID_ImportFile);
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
        GeneralPage::instance->Show(windowSelect->GetValue() == "General");
        PropPage::instance->Show(windowSelect->GetValue() == "Prop File");
        BladesPage::instance->Show(windowSelect->GetValue() == "Blade Arrays");
        PresetsPage::instance->Show(windowSelect->GetValue() == "Presets And Styles");
        BladeIDPage::instance->Show(windowSelect->GetValue() == "Blade Awareness");

        //GeneralPage::instance->update();
        BladeIDPage::instance->update();
        PropPage::instance->update();
        BladesPage::instance->update();
        PresetsPage::instance->update();

        FULLUPDATEWINDOW;
        if (PropPage::instance->IsShown()) {
          PropPage::instance->SetMinClientSize(wxSize(PropPage::instance->sizer->GetMinSize().GetWidth(), 0));
          master->Layout();
          SetSizerAndFit(MainWindow::instance->master);
          SetSize(wxSize(MainWindow::instance->GetSize().GetWidth(), MainWindow::instance->GetMinHeight() + PropPage::instance->GetBestVirtualSize().GetHeight()));
          SetMinSize(wxSize(MainWindow::instance->GetSize().GetWidth(), 350));
        }
      }, ID_WindowSelect);
}
void MainWindow::createToolTips() {
  TIP(applyButton, "Apply the current configuration to the selected Proffieboard.");
  TIP(devSelect, "Select the Proffieboard to connect to.\nThis will be an unrecognizable device identifier, but chances are there's only one which will show up.");
  TIP(refreshButton, "Refresh the detected boards.");
}

void MainWindow::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_GenFile, "Save Config\tCtrl+S", "Generate Config File");
  file->Append(ID_ExportFile, "Export Config...", "Choose a location to save a copy of your config...");
  file->Append(ID_ImportFile, "Import Config...", "Choose a file to import...");
  file->AppendSeparator();
  file->Append(ID_VerifyConfig, "Verify Config...\tCtrl+R", "Generate Config and Compile to test...");
  file->AppendSeparator();
  file->Append(ID_Initialize, "Install Dependencies...", "Install Platform-Specific Proffieboard Dependencies");
  file->AppendSeparator();
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, "Copyright Notice...");
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

void MainWindow::createPages() {
  master = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
  windowSelect = new wxComboBox(this, ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles", "Blade Awareness"  /*, "Hardware"*/}), wxCB_READONLY | wxCB_DROPDOWN);
  refreshButton = new wxButton(this, ID_RefreshDev, "Refresh...", wxDefaultPosition, wxDefaultSize, 0);
  devSelect = new wxComboBox(this, ID_DeviceSelect, "Select Device...", wxDefaultPosition, wxDefaultSize, Misc::createEntries(Arduino::getBoards()), wxCB_READONLY);
  applyButton = new wxButton(this, ID_ApplyChanges, "Apply to Board...", wxDefaultPosition, wxDefaultSize, 0);
  applyButton->Disable();
  options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->AddStretchSpacer(1);
  options->Add(refreshButton, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(devSelect, wxSizerFlags(0).Border(wxALL, 10));
  options->Add(applyButton, wxSizerFlags(0).Border(wxALL, 10));

  GeneralPage::instance = new GeneralPage(this);
  PropPage::instance = new PropPage(this);
  PresetsPage::instance = new PresetsPage(this);
  BladesPage::instance = new BladesPage(this);
  BladeIDPage::instance = new BladeIDPage(this);

  //GeneralPage::instance->update();
  PropPage::instance->update();
  PresetsPage::instance->update();
  BladesPage::instance->update();
  BladeIDPage::instance->update();

  PropPage::instance->Show(false);
  BladesPage::instance->Show(false);
  PresetsPage::instance->Show(false);
  BladeIDPage::instance->Show(false);

  master->Add(options, wxSizerFlags(0).Expand());
  master->Add(GeneralPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(PropPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(PresetsPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(BladesPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());
  master->Add(BladeIDPage::instance, wxSizerFlags(1).Border(wxALL, 10).Expand());

  SetSizerAndFit(master); // use the sizer for layout and set size and hints
}
