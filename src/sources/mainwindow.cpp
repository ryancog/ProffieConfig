#include "mainwindow.h"

MainWindow::MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {
    CreateMenuBar();
    CreatePages();
    SetMinClientSize(wxSize(0, 0));
    BindEvents();

    setConfigDefaults();
}

void MainWindow::BindEvents() {
    Bind(wxEVT_COMBOBOX, [=](wxCommandEvent&) {
        // TODO general->update();
        if (windowSelect->GetValue() == "General") {
            general->Show(true);
            presets->Show(false);
            blades->Show(false);
            hardware->Show(false);
        } else if (windowSelect->GetValue() == "Presets") {
            general->Show(false);
            presets->Show(true);
            blades->Show(false);
            hardware->Show(false);
            PresetsPage::update();
        } else if (windowSelect->GetValue() == "Blades") {
            general->Show(false);
            presets->Show(false);
            blades->Show(true);
            hardware->Show(false);
            BladesPage::update();
        } else if (windowSelect->GetValue() == "Hardware") {
            general->Show(false);
            presets->Show(false);
            blades->Show(false);
            hardware->Show(true);
        }
        UPDATEWINDOW
    }, Misc::ID_WindowSelect);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, [=](wxCommandEvent&) { Configuration::outputConfig(); }, Misc::ID_GenFile);
    Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) { Configuration::updatePresetsConfig(); PresetsPage::update(); }, Misc::ID_BladeList);
    Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) { Configuration::updatePresetsConfig(); PresetsPage::update(); }, Misc::ID_PresetList);
    Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
        // Update Style Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
            Configuration::presets[PresetsPage::settings.presetList->GetSelection()].styles[PresetsPage::settings.bladeList->GetSelection()] = PresetsPage::settings.presetsEditor->GetValue();
        } else PresetsPage::settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));

        Configuration::updatePresetsConfig();
        PresetsPage::update();
    }, Misc::ID_PresetEditor);
    Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
        // Update Name Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
            Configuration::presets[PresetsPage::settings.presetList->GetSelection()].name = PresetsPage::settings.nameInput->GetValue();
        } else PresetsPage::settings.nameInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updatePresetsConfig(); PresetsPage::update();
    }, Misc::ID_PresetName);
    Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
        // Update Dir Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
            Configuration::presets[PresetsPage::settings.presetList->GetSelection()].dirs = PresetsPage::settings.dirInput->GetValue();
        } else PresetsPage::settings.dirInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updatePresetsConfig(); PresetsPage::update();
    }, Misc::ID_PresetDir);
    Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
        // Update Track Config
        if (PresetsPage::settings.presetList->GetSelection() >= 0 && PresetsPage::settings.bladeList->GetSelection() >= 0) {
            Configuration::presets[PresetsPage::settings.presetList->GetSelection()].track = PresetsPage::settings.trackInput->GetValue();
        } else PresetsPage::settings.trackInput->ChangeValue(wxString::FromUTF8(""));

        Configuration::updatePresetsConfig(); PresetsPage::update();
    }, Misc::ID_PresetTrack);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        Configuration::presets.push_back(Configuration::Configuration::presetConfig());
        Configuration::presets[Configuration::presets.size() - 1].name = "New Preset";

        Configuration::updatePresetsConfig();
        PresetsPage::update();
    }, Misc::ID_AddPreset);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        if (PresetsPage::settings.presetList->GetSelection() >= 0)
            Configuration::presets.erase(std::next(Configuration::presets.begin(), PresetsPage::settings.presetList->GetSelection()));

        Configuration::updatePresetsConfig();
        PresetsPage::update();
    }, Misc::ID_RemovePreset);
    Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_BladeSelect);
    Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_SubBladeSelect);
    Bind(wxEVT_COMBOBOX, [=](wxCommandEvent&) {
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW;
    }, Misc::ID_BladeType);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        if (BD_HASSELECTION) {
            Configuration::blades.insert(Configuration::blades.begin() + BladesPage::lastBladeSelection + 1, Configuration::Configuration::bladeConfig());
        } else {
            Configuration::blades.push_back(Configuration::Configuration::bladeConfig());
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_AddBlade);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        Configuration::blades[BladesPage::lastBladeSelection].isSubBlade = true;
        Configuration::blades[BladesPage::lastBladeSelection].subBlades.push_back(Configuration::Configuration::bladeConfig::subBladeInfo());
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_AddSubBlade);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        if (BD_HASSELECTION) {
            Configuration::blades.erase(Configuration::blades.begin() + BladesPage::lastBladeSelection);
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_RemoveBlade);
    Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        if (BD_SUBHASSELECTION) {
            Configuration::blades[BladesPage::lastBladeSelection].subBlades.erase(Configuration::blades[BladesPage::lastBladeSelection].subBlades.begin() + BladesPage::lastSubBladeSelection);
            if (Configuration::blades[BladesPage::lastBladeSelection].subBlades.size() < 1) Configuration::blades[BladesPage::lastBladeSelection].isSubBlade = false;
            BladesPage::lastSubBladeSelection = -1;
        }
        Configuration::updateBladesConfig();
        BladesPage::update();
        UPDATEWINDOW
    }, Misc::ID_RemoveSubBlade);
}

void MainWindow::setConfigDefaults() {
    Configuration::presets.push_back(Configuration::Configuration::presetConfig());
    Configuration::presets[0].name = "My First Preset";
    Configuration::presets[0].dirs = "smthjedi";
    Configuration::presets[0].track = "tracks/track1.wav";
    Configuration::presets[0].styles.push_back("StylePtr<Black>()");

    Configuration::blades.push_back(Configuration::Configuration::bladeConfig());
}

void MainWindow::CreateMenuBar() {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);
    menuFile->Append(wxID_ABOUT);

    wxMenu *menuConfig = new wxMenu;
    menuConfig->Append(Misc::ID_GenFile, "&Generate Config\t", "Generate Config File");

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuConfig, "&Config");
    SetMenuBar(menuBar);
}

void MainWindow::CreatePages() {
    master = new wxBoxSizer(wxVERTICAL);
    windowSelect = new wxComboBox(this, Misc::ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, {"General", "Presets", "Blades", "Hardware"}, wxCB_READONLY);

    general = new GeneralPage(this);
    presets = new PresetsPage(this);
    blades = new BladesPage(this);
    hardware = new HardwarePage(this);

    presets->Show(false);
    blades->Show(false);
    hardware->Show(false);

    master->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));
    master->Add(general, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
    master->Add(presets, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
    master->Add(blades, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
    master->Add(hardware, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());

    SetSizerAndFit(master); // use the sizer for layout and set size and hints
}

void MainWindow::OnExit(wxCommandEvent&) { Close(true); }

void MainWindow::OnAbout(wxCommandEvent&) { wxMessageBox("Tool for GUI Configuration::getConfig()uration and flashing of ProffieBoard\n\nCreated by Ryryog25", "About ProffieConfig", wxOK | wxICON_INFORMATION); }
