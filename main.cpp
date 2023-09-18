// wxWidgets "Hello, World!"

#include <string>
#include <vector>
#include <fstream>

#include <wx/app.h>
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/addremovectrl.h>
#include <wx/notebook.h>
#include <wx/utils.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "defines.h"


class ProffieConfig : public wxApp {
public:
    virtual bool OnInit();
};

class MyDialog : wxDialog {
    MyDialog(wxFrame *parent, wxWindowID id, const wxString &title );
};

class MainWindow : public wxFrame {
public:
    MainWindow() : wxFrame(NULL, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize) {

        // Menus and MenuBar
        {
            wxMenu *menuFile = new wxMenu;
            menuFile->Append(wxID_EXIT);
            menuFile->Append(wxID_ABOUT);

            wxMenu *menuConfig = new wxMenu;
            menuConfig->Append(ID_GenFile, "&Generate Config\t", "Generate Config File");

            wxMenuBar *menuBar = new wxMenuBar;
            menuBar->Append(menuFile, "&File");
            menuBar->Append(menuConfig, "&Config");
            SetMenuBar(menuBar);
        }


        config.presets.push_back(presetConfig());
        config.presets[0].name = "My First Preset";
        config.presets[0].dirs = "smthjedi";
        config.presets[0].track = "tracks/track1.wav";
        config.presets[0].styles.push_back("StylePtr<Black>()");

        config.blades.push_back(bladeConfig());

        // Set up UI Elements
        wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);
        //notebook = new wxNotebook(this, ID_Notebook);
        wxComboBox *windowSelect = new wxComboBox(this, ID_WindowSelect, "General", wxDefaultPosition, wxDefaultSize, {"General", "Presets", "Blades", "Hardware"}, wxCB_READONLY);

        createGeneral();
        createPresets();
        createBlades();
        createHardware();

        presets->Show(false);
        blades->Show(false);
        hardware->Show(false);

        master->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));
        master->Add(general, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
        master->Add(presets, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
        master->Add(blades, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
        master->Add(hardware, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());

        SetSizerAndFit(master); // use the sizer for layout and set size and hints
        SetMinClientSize(wxSize(0, 0));

        Bind(wxEVT_COMBOBOX, [=](wxCommandEvent&) {
            updateGeneralPage();
            updatePresetsPage();
            updateBladesPage();

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
                updatePresetsPage();
            } else if (windowSelect->GetValue() == "Blades") {
                general->Show(false);
                presets->Show(false);
                blades->Show(true);
                hardware->Show(false);
                updateBladesPage();
            } else if (windowSelect->GetValue() == "Hardware") {
                general->Show(false);
                presets->Show(false);
                blades->Show(false);
                hardware->Show(true);
            }
            UPDATEWINDOW
        }, ID_WindowSelect);
        Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
        Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, [=](wxCommandEvent&) { outputConfig(); }, ID_GenFile);
        Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) { updatePresetsPage(); }, ID_BladeList);
        Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) { updatePresetsPage(); }, ID_PresetList);
        Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
            if (settings.presetList->GetSelection() >= 0 && settings.bladeList->GetSelection() >= 0) {
                config.presets[settings.presetList->GetSelection()].styles[settings.bladeList->GetSelection()] = settings.presetsEditor->GetValue();
            } else settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));
            updatePresetsPage();
        }, ID_PresetEditor);
        Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
            if (settings.presetList->GetSelection() >= 0 && settings.bladeList->GetSelection() >= 0) {
                config.presets[settings.presetList->GetSelection()].name = settings.nameInput->GetValue();
            } else settings.nameInput->ChangeValue(wxString::FromUTF8(""));

            updatePresetsPage();
        }, ID_PresetName);
        Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
            if (settings.presetList->GetSelection() >= 0 && settings.bladeList->GetSelection() >= 0) {
                config.presets[settings.presetList->GetSelection()].dirs = settings.dirInput->GetValue();
            } else settings.dirInput->ChangeValue(wxString::FromUTF8(""));
            updatePresetsPage();
        }, ID_PresetDir);
        Bind(wxEVT_TEXT, [=](wxCommandEvent&) {
            if (settings.presetList->GetSelection() >= 0 && settings.bladeList->GetSelection() >= 0) {
                config.presets[settings.presetList->GetSelection()].track = settings.trackInput->GetValue();
            } else settings.trackInput->ChangeValue(wxString::FromUTF8(""));
            updatePresetsPage();
        }, ID_PresetTrack);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            config.presets.push_back(presetConfig());
            config.presets[config.presets.size() - 1].name = "New Preset";
            updatePresetsPage();
        }, ID_AddPreset);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            if (settings.presetList->GetSelection() >= 0) config.presets.erase(std::next(config.presets.begin(),settings.presetList->GetSelection()));
            updatePresetsPage();
        }, ID_RemovePreset);
        Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) {
            updateBladesPage();
            UPDATEWINDOW
        }, ID_BladeSelect);
        Bind(wxEVT_LISTBOX, [=](wxCommandEvent&) {
            updateBladesPage();
            UPDATEWINDOW
        }, ID_SubBladeSelect);
        Bind(wxEVT_COMBOBOX, [=](wxCommandEvent&) {
            updateBladesPage();
            UPDATEWINDOW;
        }, ID_BladeType);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            if (BD_HASSELECTION) {
                config.blades.insert(config.blades.begin() + lastBladeSelection + 1, bladeConfig());
            } else {
                config.blades.push_back(bladeConfig());
            }
            updateBladesPage();
            UPDATEWINDOW
        }, ID_AddBlade);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            config.blades[lastBladeSelection].isSubBlade = true;
            config.blades[lastBladeSelection].subBlades.push_back(bladeConfig::subBladeInfo());
            updateBladesPage();
            UPDATEWINDOW
        }, ID_AddSubBlade);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            if (BD_HASSELECTION) {
                config.blades.erase(config.blades.begin() + lastBladeSelection);
            }
            updateBladesPage();
            UPDATEWINDOW
        }, ID_RemoveBlade);
        Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            if (BD_SUBHASSELECTION) {
                config.blades[lastBladeSelection].subBlades.erase(config.blades[lastBladeSelection].subBlades.begin() + lastSubBladeSelection);
                if (config.blades[lastBladeSelection].subBlades.size() < 1) config.blades[lastBladeSelection].isSubBlade = false;
                lastSubBladeSelection = -1;
            }
            updateBladesPage();
            UPDATEWINDOW
        }, ID_RemoveSubBlade);
    }

private:

    enum {
        ID_WindowSelect,
        ID_GenFile,
        ID_PresetList,
        ID_BladeList,
        ID_PresetEditor,
        ID_AddPreset,
        ID_RemovePreset,
        ID_PresetName,
        ID_PresetDir,
        ID_PresetTrack,
        ID_BladeSelect,
        ID_SubBladeSelect,
        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSubBlade,
        ID_RemoveSubBlade,
        ID_BladeType,
    };

    enum class SABERPROP {
        DEFAULT,
        SA22C,
        FETT263,
        SHTOK,
        BC
    };

    enum class PROFFIEBOARD {
        V1,
        V2,
        V3
    };

    enum class ORIENTATION {
        FETS_TOWARDS_BLADE,
        USB_TOWARDS_BLADE,
        TOP_TOWARDS_BLADE,
        BOTTOM_TOWARDS_BLADE,
        CUSTOM
    };

    enum class BLADETYPE {
        NEOPIXEL_3,
        NEOPIXEL_4,
        CREE_3,
        CREE_4,
        SINGLECOLOR,
        NONE
    };

    enum class C_ORDER {
        BGR,
        BRG,
        GBR,
        GRB,
        RBG,
        RGB,
        BGRW,
        BRGW,
        GBRW,
        GRBW,
        RBGW,
        RGBW,
        WBGR,
        WBRG,
        WGBR,
        WGRB,
        WRBG,
        WRGB,
        BGRw,
        BRGw,
        GBRw,
        GRBw,
        RBGw,
        RGBw,
        wBGR,
        wBRG,
        wGBR,
        wGRB,
        wRBG,
        wRGB
    };

    enum class CREETYPE {
        RED,
        GREEN,
        PCAMBER,
        AMBER,
        BLUE,
        REDORANGE,
        WHITE,
        XPL,
        NOLED
    };

    struct numEntry {
        wxBoxSizer *box{nullptr};
        wxSpinCtrl *num{nullptr};
        wxSpinCtrlDouble *doubleNum{nullptr};
    };

    struct bladeConfig {
        std::string type{"NeoPixel (RGB)"};

        std::string dataPin{"Pin 1"};
        std::string colorType{"GRB"};
        int numPixels{144};
        bool useRGBWithWhite{false};

        std::string Cree1{"Red"};
        std::string Cree2{"Green"};
        std::string Cree3{"Blue"};
        std::string Cree4{"White"};
        int Cree1Resistance{1000};
        int Cree2Resistance{0};
        int Cree3Resistance{240};
        int Cree4Resistance{550};

        bool usePowerPin1{false};
        bool usePowerPin2{false};
        bool usePowerPin3{false};
        bool usePowerPin4{false};
        bool usePowerPin5{false};
        bool usePowerPin6{false};

        bool isSubBlade{false};
        bool subBladeWithStride{false};

        struct subBladeInfo {
            int startPixel{0};
            int endPixel{0};
        };
        std::vector<subBladeInfo> subBlades{};
    };

    struct presetConfig {
        std::vector<std::string> styles{};
        std::string name{""};
        std::string dirs{""};
        std::string track{""};
    };

    wxStaticBoxSizer *general{nullptr};
    wxStaticBoxSizer *presets{nullptr};
    wxStaticBoxSizer *blades{nullptr};
    wxStaticBoxSizer *hardware{nullptr};
    void createGeneral();
    void createPresets();
    void createBlades();
    void createHardware();

    void updateGeneralPage();
    void updatePresetsPage();
    void updateBladesPage();

    int lastBladeSelection = -1;
    int lastSubBladeSelection = -1;


    numEntry createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int ID, int minVal, int maxVal, int defaultVal);
    numEntry createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int ID, double minVal, double maxVal, double defaultVal);

    CREETYPE stringToCreeType(std::string);
    std::string creeTypeToString(CREETYPE);
    BLADETYPE stringToBladeType(std::string);
    std::string bladeTypeToString(BLADETYPE);
    C_ORDER stringToColorOrder(std::string, bool);
    std::string colorOrder3ToString(C_ORDER);
    std::string colorOrder4ToString(C_ORDER);
    bool colorOrderUseRGB(C_ORDER);
    int stringToDataPin(std::string);
    std::string dataPinToString(int);


    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnButton(wxCommandEvent& event);

    void outputConfig();

    struct {
        wxComboBox *board{nullptr};
        wxCheckBox *massStorage{nullptr};
        wxCheckBox *webUSB{nullptr};

        wxComboBox *prop{nullptr};
        wxCheckBox *noLockupHold{nullptr};
        wxCheckBox *stabOn{nullptr};
        wxCheckBox *swingOn{nullptr};
        wxCheckBox *twistOn{nullptr};
        wxCheckBox *thrustOn{nullptr};
        wxCheckBox *twistOff{nullptr};

        //wxCheckBox *battleEnable{nullptr};
        wxCheckBox *gestureEnBattle{nullptr};
        numEntry lockupDelay;
        wxCheckBox *forcePush{nullptr};
        numEntry forcePushLength;

        wxTextCtrl *presetsEditor{nullptr};
        wxListBox *presetList{nullptr};
        wxListBox *bladeList{nullptr};
        wxButton *addPreset{nullptr};
        wxButton *removePreset{nullptr};

        wxTextCtrl *nameInput{nullptr};
        wxTextCtrl *dirInput{nullptr};
        wxTextCtrl *trackInput{nullptr};

        numEntry buttons;
        numEntry volume;
        numEntry clash;
        numEntry pliTime;
        numEntry idleTime;
        numEntry motion;
        wxCheckBox *volumeSave{nullptr};
        wxCheckBox *presetSave{nullptr};
        wxCheckBox *colorSave{nullptr};
        wxCheckBox *disableColor{nullptr};
        wxCheckBox *disableDev{nullptr};

        wxListBox *bladeSelect{nullptr};
        wxListBox *subBladeSelect{nullptr};
        wxButton *addBlade{nullptr};
        wxButton *removeBlade{nullptr};
        wxButton *addSubBlade{nullptr};
        wxButton *removeSubBlade{nullptr};

        wxComboBox *bladeType{nullptr};
        wxStaticText *bladeDataPinLabel{nullptr};
        wxComboBox *bladeDataPin{nullptr};
        wxStaticText *bladePixelsLabel{nullptr};
        wxSpinCtrl *bladePixels{nullptr};

        wxCheckBox *usePowerPin1{nullptr};
        wxCheckBox *usePowerPin2{nullptr};
        wxCheckBox *usePowerPin3{nullptr};
        wxCheckBox *usePowerPin4{nullptr};
        wxCheckBox *usePowerPin5{nullptr};
        wxCheckBox *usePowerPin6{nullptr};

        wxStaticText *bladeColorOrderLabel{nullptr};
        wxComboBox *blade3ColorOrder{nullptr};
        wxComboBox *blade4ColorOrder{nullptr};
        wxCheckBox *blade4UseRGB{nullptr};
        wxStaticText *star1ColorLabel{nullptr};
        wxComboBox *star1Color{nullptr};
        wxStaticText *star2ColorLabel{nullptr};
        wxComboBox *star2Color{nullptr};
        wxStaticText *star3ColorLabel{nullptr};
        wxComboBox *star3Color{nullptr};
        wxStaticText *star4ColorLabel{nullptr};
        wxComboBox *star4Color{nullptr};

        wxCheckBox *bladeDetect{nullptr};
        wxStaticText *bladeDetectPinLabel{nullptr};
        wxComboBox *bladeDetectPin{nullptr};

        wxCheckBox *subBladeUseStride{nullptr};
        wxStaticText *subBladeStartLabel{nullptr};
        wxSpinCtrl *subBladeStart{nullptr};
        wxStaticText *subBladeEndLabel{nullptr};
        wxSpinCtrl *subBladeEnd{nullptr};

    } settings;

    struct {
        struct {
            bool audioEnabled{true};
            bool motionEnabled{true};
            bool ws2811Enabled{true};
            bool sdEnabled{true};
            bool disableBasicStyles{true};
        } defaults;

        struct {
            PROFFIEBOARD board{PROFFIEBOARD::V2};
            bool massStorage{false};
            bool webUSB{false};
        } boardConfig;

        struct {
            SABERPROP prop{SABERPROP::SA22C};
            bool stabOn{false};
            bool swingOn{false};
            bool twistOn{false};
            bool thrustOn{false};
            bool twistOff{false};
            bool battleMode{false};
            bool gestureBattle{false};
            bool revertLockup{false};
            bool forcePush{false};
            int forcePushLength{5};
            int lockupDelay{200};
        } propConfig;

        struct {
            int numButtons{2};
            int volume{2000};
            double clashThreshold{3};

            int maxLEDs{144};
        } general;

        struct {
            int saveVolume{true};
            int savePreset{true};
            int saveColorChange{true};
            int disableColorChange{false};
        } options;

        struct {
            bool hasOLED{false};
            bool hasBLE{false};
            bool hasRFID{false};
            bool bladeDetect{false};
            std::string bladeDetectPin{""};
        } features;

        struct {
            int idleTimout{10 * 60};
            int pliTimeout{2 * 60};
            int motionTimeout{15 * 60};
            bool devCommands{false};

            ORIENTATION orientation{ORIENTATION::FETS_TOWARDS_BLADE};
            struct {
                int x{0};
                int y{0};
                int z{0};
            } orientationRotation;

            bool dualPowerButtons{false};
        } tweaks;

        std::vector<presetConfig> presets{};
        std::vector<bladeConfig> blades{};

    } config;
};

void MainWindow::createGeneral() {
    // Sizers
    general = new wxStaticBoxSizer(wxVERTICAL, this, "General");
    wxBoxSizer *generalHoriz = new wxBoxSizer(wxHORIZONTAL);
    // Can't access the point members for some reason unless we do this:

    wxStaticBoxSizer *boardSetup = new wxStaticBoxSizer(wxVERTICAL, general->GetStaticBox(), "Board Setup");
    wxStaticBoxSizer *propSetup = new wxStaticBoxSizer(wxHORIZONTAL, general->GetStaticBox(), "Prop Setup");
    wxStaticBoxSizer *options = new wxStaticBoxSizer(wxHORIZONTAL, general->GetStaticBox(), "Options");

    wxWrapSizer *propSetup1 = new wxWrapSizer(wxVERTICAL, 0);
    wxWrapSizer *options1 = new wxWrapSizer(wxVERTICAL, 0);

    wxStaticBoxSizer *battleMode = new wxStaticBoxSizer(wxVERTICAL, propSetup->GetStaticBox(), "Battle Mode");

    // boardSetup
    {
        settings.board = new wxComboBox(boardSetup->GetStaticBox(), wxID_ANY, "ProffieBoard V2", wxDefaultPosition, wxDefaultSize, {"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}, wxCB_READONLY);
        settings.massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
        settings.webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

        boardSetup->Add(settings.board, wxSizerFlags(0).Border(wxALL, 10));
        boardSetup->Add(settings.massStorage, wxSizerFlags(0).Border(wxALL, 10));
        boardSetup->Add(settings.webUSB, wxSizerFlags(0).Border(wxALL, 10));
    }

    // propSetup
    {
        // General
        settings.prop = new wxComboBox(propSetup->GetStaticBox(), wxID_ANY, "SA22C", wxDefaultPosition, wxDefaultSize, {"Default", "SA22C", "FETT263", "Shtok", "NoSloppy"}, wxCB_READONLY);
        settings.noLockupHold = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Revert Lockup and Multi-Blast Trigger");
        // Add option when reading out buttons
        settings.stabOn = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Stab To Turn On");
        settings.swingOn = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Swing To Turn On");
        settings.twistOn = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Twist To Turn On");
        settings.thrustOn = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Thrust To Turn On");
        settings.twistOff = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Twist To Turn Off");

        // Battle Mode
        //settings.battleEnable = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Enable Battle Mode");
        settings.gestureEnBattle = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Gesture Ignition Starts Battle Mode");

        settings.lockupDelay = createNumEntry(battleMode, "Lockup Delay (ms)", wxID_ANY, 0, 3000, 200);

        // force push
        settings.forcePush = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Enable Force Push");
        settings.forcePushLength = createNumEntry(propSetup, "Force Push Length", wxID_ANY, 0, 10, 5);

        // Prop Setup 1
        propSetup1->Add(settings.prop, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.stabOn, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.swingOn, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.twistOn, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.thrustOn, MENUITEMDEFAULTFLAGS);
        // Prop Setup 2
        //propSetup2->Add(settings.battleEnable, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.gestureEnBattle, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.noLockupHold, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.twistOff, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.forcePush, MENUITEMDEFAULTFLAGS);
        propSetup1->Add(settings.forcePushLength.box, MENUITEMDEFAULTFLAGS);
        // Battle Mode
        battleMode->Add(settings.lockupDelay.box, wxSizerFlags(0).Border(wxALL, 10));
        propSetup1->Add(battleMode, wxSizerFlags(0).Border(wxALL, 10));

        propSetup->Add(propSetup1, wxSizerFlags(1));
    }

    // Options
    {
        settings.buttons = createNumEntry(options, "Number of Buttons", wxID_ANY, 1, 3, 2);
        settings.volume = createNumEntry(options, "Max Volume", wxID_ANY, 0, 3500, 2000);
        settings.clash = createNumEntryDouble(options, "Clash Threshold", wxID_ANY, 0.1, 5, 3);
        settings.pliTime = createNumEntry(options, "PLI Timeout", wxID_ANY, 1, 60, 2);
        settings.idleTime = createNumEntry(options, "Idle Timeout", wxID_ANY, 1, 60, 10);
        settings.motion = createNumEntry(options, "Motion Timeout", wxID_ANY, 1, 60, 15);

        settings.volumeSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Volume");
        settings.presetSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Preset");
        settings.colorSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Color");
        settings.disableColor = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Color Change");
        settings.disableDev = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Developer Commands");

        // Options 1
        options1->Add(settings.buttons.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.volume.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.clash.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.pliTime.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.idleTime.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.motion.box, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.volumeSave, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.presetSave, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.colorSave, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.disableColor, MENUITEMDEFAULTFLAGS);
        options1->Add(settings.disableDev, MENUITEMDEFAULTFLAGS);

        options->Add(options1);
    }

    generalHoriz->Add(boardSetup, wxSizerFlags(/*proportion*/ 2).Border(wxALL, 10).Expand());
    generalHoriz->Add(propSetup, wxSizerFlags(/*proportion*/ 7).Border(wxALL, 10).Expand());
    general->Add(generalHoriz, wxSizerFlags(10).Expand());
    general->Add(options, wxSizerFlags(8).Border(wxALL, 10).Expand());
}

void MainWindow::createPresets() {
    presets = new wxStaticBoxSizer(wxHORIZONTAL, this, "Presets");
    wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);
    settings.presetsEditor = new wxTextCtrl(presets->GetStaticBox(), ID_PresetEditor, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
    settings.presetsEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxBoxSizer *presetsConfig = new wxBoxSizer(wxVERTICAL);

    // Preset Select
    {
        wxBoxSizer *presetLists = new wxBoxSizer(wxHORIZONTAL);

        settings.presetList = new wxListBox(presets->GetStaticBox(), ID_PresetList);
        settings.bladeList = new wxListBox(presets->GetStaticBox(), ID_BladeList);
        presetLists->Add(settings.presetList, wxSizerFlags(1).Expand());
        presetLists->Add(settings.bladeList, wxSizerFlags(1).Expand());

        wxBoxSizer *presetButtons = new wxBoxSizer(wxHORIZONTAL);
        settings.addPreset = new wxButton(presets->GetStaticBox(), ID_AddPreset, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        settings.removePreset = new wxButton(presets->GetStaticBox(), ID_RemovePreset, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        presetButtons->Add(settings.addPreset, wxSizerFlags(0).Border(wxRIGHT, 10));
        presetButtons->Add(settings.removePreset);

        presetSelect->Add(presetLists, wxSizerFlags(1));
        presetSelect->Add(presetButtons, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 5));
    }

    // Preset Config
    {
        wxStaticText *name = new wxStaticText(presets->GetStaticBox(), wxID_ANY, "Preset Name", wxDefaultPosition, wxSize(150, 20));
        settings.nameInput = new wxTextCtrl(presets->GetStaticBox(), ID_PresetName);
        wxStaticText *dir = new wxStaticText(presets->GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
        settings.dirInput = new wxTextCtrl(presets->GetStaticBox(), ID_PresetDir);
        wxStaticText *track = new wxStaticText(presets->GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
        settings.trackInput = new wxTextCtrl(presets->GetStaticBox(), ID_PresetTrack);

        presetsConfig->Add(name, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        presetsConfig->Add(settings.nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());
        presetsConfig->Add(dir, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        presetsConfig->Add(settings.dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());
        presetsConfig->Add(track, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        presetsConfig->Add(settings.trackInput, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM, 10).Expand());
    }

    presets->Add(presetsConfig, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
    presets->Add(presetSelect, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10).Expand());
    presets->Add(settings.presetsEditor, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
}

void MainWindow::createBlades() {
    blades = new wxStaticBoxSizer(wxHORIZONTAL, this, "Blades");

    wxBoxSizer *bladeManager = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bladeSelection = new wxBoxSizer(wxVERTICAL);
    wxStaticText *bladeText = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Blades");
    settings.bladeSelect = new wxListBox(blades->GetStaticBox(), ID_BladeSelect);
    wxBoxSizer *bladeButtons = new wxBoxSizer(wxHORIZONTAL);
    settings.addBlade = new wxButton(blades->GetStaticBox(), ID_AddBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    settings.removeBlade = new wxButton(blades->GetStaticBox(), ID_RemoveBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    bladeButtons->Add(settings.addBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
    bladeButtons->Add(settings.removeBlade, wxSizerFlags(0).Border(wxTOP, 10));
    bladeSelection->Add(bladeText, wxSizerFlags(0));
    bladeSelection->Add(settings.bladeSelect, wxSizerFlags(1).Expand());
    bladeSelection->Add(bladeButtons, wxSizerFlags(0));

    wxBoxSizer *subBladeSelection = new wxBoxSizer(wxVERTICAL);
    wxStaticText *subBladeText = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "SubBlades");
    settings.subBladeSelect = new wxListBox(blades->GetStaticBox(), ID_SubBladeSelect);
    wxBoxSizer *subBladeButtons = new wxBoxSizer(wxHORIZONTAL);
    settings.addSubBlade = new wxButton(blades->GetStaticBox(), ID_AddSubBlade, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    settings.removeSubBlade = new wxButton(blades->GetStaticBox(), ID_RemoveSubBlade, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    subBladeButtons->Add(settings.addSubBlade, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 10));
    subBladeButtons->Add(settings.removeSubBlade, wxSizerFlags(0).Border(wxTOP, 10));
    subBladeSelection->Add(subBladeText, wxSizerFlags(0));
    subBladeSelection->Add(settings.subBladeSelect, wxSizerFlags(1).Expand());
    subBladeSelection->Add(subBladeButtons, wxSizerFlags(0));

    bladeManager->Add(bladeSelection, wxSizerFlags(0).Expand());
    bladeManager->Add(subBladeSelection, wxSizerFlags(0).Expand());


    wxWrapSizer *bladeSetup = new wxWrapSizer(wxVERTICAL);
    settings.bladeType = new wxComboBox(blades->GetStaticBox(), ID_BladeType, "NeoPixel (RGB)", wxDefaultPosition, wxDefaultSize, {"NeoPixel (RGB)", "NeoPixel (RGBW)", "Tri-Star Cree", "Quad-Star Cree", "Single Color"}, wxCB_READONLY);
    settings.usePowerPin1 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 1");
    settings.usePowerPin2 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 2");
    settings.usePowerPin3 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 3");
    settings.usePowerPin4 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 4");
    settings.usePowerPin5 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 5");
    settings.usePowerPin6 = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Power Pin 6");
    bladeSetup->Add(settings.bladeType, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin1, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin2, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin3, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin4, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin5, MENUITEMDEFAULTFLAGS);
    bladeSetup->Add(settings.usePowerPin6, MENUITEMDEFAULTFLAGS);

    wxWrapSizer *bladeSettings = new wxWrapSizer(wxVERTICAL);
    settings.bladeColorOrderLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Color Order");
    settings.blade3ColorOrder = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "GRB", wxDefaultPosition, wxDefaultSize, {"BGR", "BRG", "GBR", "GRB", "RBG", "RGB"}, wxCB_READONLY);
    settings.blade4ColorOrder = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "GRBW", wxDefaultPosition, wxDefaultSize, {"BGRW", "BRGW", "GBRW", "GRBW", "RBGW", "RGBW", "WBGR", "WBRG", "WGBR", "WGRB", "WRBG", "WRGB"}, wxCB_READONLY);
    settings.blade4UseRGB = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use RGB with White");
    settings.star1ColorLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Star 1 Color");
    settings.star1Color = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    settings.star2ColorLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Star 2 Color");
    settings.star2Color = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    settings.star3ColorLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Star 3 Color");
    settings.star3Color = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);
    settings.star4ColorLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Star 4 Color");
    settings.star4Color = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "<None>", wxDefaultPosition, wxDefaultSize, {"Red", "Green", "Blue", "Amber", "RedOrange", "White", "<None>"}, wxCB_READONLY);

    settings.bladeDataPinLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Blade Data Pin");
    settings.bladeDataPin = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "Pin 1", wxDefaultPosition, wxDefaultSize, {"Pin 1", "Pin 2", "Pin 3", "Pin 4"});
    settings.bladePixelsLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Number of Pixels");
    settings.bladePixels = new wxSpinCtrl(blades->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);

    settings.bladeDetect = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Blade Detect");
    settings.bladeDetectPinLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "Blade Detect Pin");
    settings.bladeDetectPin = new wxComboBox(blades->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, {"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"});

    settings.subBladeUseStride = new wxCheckBox(blades->GetStaticBox(), wxID_ANY, "Use Stride for SubBlade");
    settings.subBladeStartLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "SubBlade Start");
    settings.subBladeStart = new wxSpinCtrl(blades->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
    settings.subBladeEndLabel = new wxStaticText(blades->GetStaticBox(), wxID_ANY, "SubBlade End");
    settings.subBladeEnd = new wxSpinCtrl(blades->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 144, 0);
    //settings.bladeID

    bladeSettings->Add(settings.bladeColorOrderLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.blade3ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.blade4ColorOrder, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.blade4UseRGB, MENUITEMDEFAULTFLAGS);
    bladeSettings->Add(settings.star1ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star1Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star2ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star2Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star3ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star3Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star4ColorLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.star4Color, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    bladeSettings->Add(settings.bladeDataPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladeDataPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladePixelsLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladePixels, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    bladeSettings->Add(settings.bladeDetect, MENUITEMDEFAULTFLAGS);
    bladeSettings->Add(settings.bladeDetectPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.bladeDetectPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeUseStride, MENUITEMDEFAULTFLAGS);
    bladeSettings->Add(settings.subBladeStartLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeStart, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeEndLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
    bladeSettings->Add(settings.subBladeEnd, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

    blades->Add(bladeManager, MENUITEMDEFAULTFLAGS);
    blades->Add(bladeSetup, wxSizerFlags(0));
    blades->Add(bladeSettings, wxSizerFlags(1));
}

void MainWindow::createHardware() {
    hardware = new wxStaticBoxSizer(wxHORIZONTAL, this, "Hardware");
}

void MainWindow::updateGeneralPage() {
    // Save Config
    config.boardConfig.board = settings.board->GetValue() == "ProffieBoard V1" ? PROFFIEBOARD::V1 : settings.board->GetValue() == "ProffieBoard V2" ? PROFFIEBOARD::V2 : PROFFIEBOARD::V3;
    config.boardConfig.massStorage = settings.massStorage->GetValue();
    config.boardConfig.webUSB = settings.webUSB->GetValue();

    config.propConfig.prop = settings.prop->GetValue() == "Default" ? SABERPROP::DEFAULT :
                                                                      settings.prop->GetValue() == "SA22C" ? SABERPROP::SA22C :
                                                                                                             settings.prop->GetValue() == "FETT263" ? SABERPROP::FETT263 :
                                                                                                                                                      settings.prop->GetValue() == "Shtok" ? SABERPROP::SHTOK :
                                                                                                                                                                                             SABERPROP::BC;
    config.propConfig.stabOn = settings.stabOn->GetValue();
    config.propConfig.swingOn = settings.swingOn->GetValue();
    config.propConfig.twistOn = settings.twistOn->GetValue();
    config.propConfig.thrustOn = settings.thrustOn->GetValue();
    config.propConfig.twistOff = settings.twistOff->GetValue();
    //config.propConfig.battleMode = settings.battleEnable->GetValue();
    config.propConfig.gestureBattle = settings.gestureEnBattle->GetValue();
    config.propConfig.revertLockup = settings.noLockupHold->GetValue();
    config.propConfig.forcePush = settings.forcePush->GetValue();
    config.propConfig.forcePushLength = settings.forcePushLength.num->GetValue();
    config.propConfig.lockupDelay = settings.lockupDelay.num->GetValue();

    config.options.saveVolume = settings.volumeSave->GetValue();
    config.options.savePreset = settings.presetSave->GetValue();
    config.options.saveColorChange = settings.colorSave->GetValue();
    config.options.disableColorChange = settings.disableColor->GetValue();

    config.tweaks.devCommands = !settings.disableDev->GetValue();
    config.general.numButtons = settings.buttons.num->GetValue();
    config.general.volume = settings.volume.num->GetValue();
    config.general.clashThreshold = settings.clash.doubleNum->GetValue();

    config.tweaks.pliTimeout = settings.pliTime.num->GetValue();
    config.tweaks.idleTimout = settings.idleTime.num->GetValue();
    config.tweaks.motionTimeout = settings.motion.num->GetValue();
}

void MainWindow::updatePresetsPage() {
    int presetIndex = settings.presetList->GetSelection();
    int bladeIndex = settings.bladeList->GetSelection();
    int listSelection = presetIndex;
    settings.presetList->Clear();
    for (presetConfig preset : config.presets) {
        settings.presetList->Append(preset.name);
    }
    if ((int)settings.presetList->GetCount() - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0) settings.presetList->SetSelection(listSelection);

    listSelection = bladeIndex;
    settings.bladeList->Clear();
    for (unsigned int blade = 0; blade < config.blades.size(); blade++) {
        if (config.blades[blade].subBlades.size() > 0) {
            for (unsigned int subBlade = 0; subBlade < config.blades[blade].subBlades.size(); subBlade++) {
                settings.bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
            }
        } else {
            settings.bladeList->Append("Blade " + std::to_string(blade));
        }
    }
    if ((int)settings.bladeList->GetCount() - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0) settings.bladeList->SetSelection(listSelection);

    for (presetConfig &preset : config.presets) {
        // Calculate # of presets there should be prior.
        int numBlades = 0;
        for (bladeConfig blade : config.blades) {
            numBlades++;
            numBlades += blade.subBlades.size();
        }

        if (numBlades > (int)preset.styles.size()) {
            while (numBlades != (int)preset.styles.size()) {
                preset.styles.push_back("StylePtr<Black>()");
            }
        } else if (numBlades < (int)preset.styles.size()) {
            while (numBlades != (int)preset.styles.size()) {
                preset.styles.pop_back();
            }
        }
    }

    presetIndex = settings.presetList->GetSelection();
    bladeIndex = settings.bladeList->GetSelection();
    if (presetIndex >= 0 && bladeIndex >= 0) {
        settings.presetsEditor->ChangeValue(wxString::FromUTF8(config.presets[presetIndex].styles[bladeIndex]));
        settings.nameInput->ChangeValue(wxString::FromUTF8(config.presets[presetIndex].name));
        settings.dirInput->ChangeValue(wxString::FromUTF8(config.presets[presetIndex].dirs));
        settings.trackInput->ChangeValue(wxString::FromUTF8(config.presets[presetIndex].track));
    }
    else {
        settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));
        settings.nameInput->ChangeValue(wxString::FromUTF8(""));
        settings.dirInput->ChangeValue(wxString::FromUTF8(""));
        settings.trackInput->ChangeValue(wxString::FromUTF8(""));
    }
}

void MainWindow::updateBladesPage() {
    if (lastBladeSelection >= 0 && lastBladeSelection < (int)config.blades.size()) { // Save Options
        config.blades[lastBladeSelection].type = settings.bladeType->GetValue();
        config.blades[lastBladeSelection].usePowerPin1 = settings.usePowerPin1->GetValue();
        config.blades[lastBladeSelection].usePowerPin2 = settings.usePowerPin2->GetValue();
        config.blades[lastBladeSelection].usePowerPin3 = settings.usePowerPin3->GetValue();
        config.blades[lastBladeSelection].usePowerPin4 = settings.usePowerPin4->GetValue();
        config.blades[lastBladeSelection].usePowerPin5 = settings.usePowerPin5->GetValue();
        config.blades[lastBladeSelection].usePowerPin6 = settings.usePowerPin6->GetValue();

        config.blades[lastBladeSelection].dataPin = settings.bladeDataPin->GetValue();
        config.blades[lastBladeSelection].numPixels = settings.bladePixels->GetValue();
        config.blades[lastBladeSelection].colorType = config.blades[lastBladeSelection].type == "NeoPixel (RGB)" ? settings.blade3ColorOrder->GetValue() : settings.blade4ColorOrder->GetValue();
        config.blades[lastBladeSelection].useRGBWithWhite = settings.blade4UseRGB->GetValue();

        config.blades[lastBladeSelection].Cree1 = settings.star1Color->GetValue();
        config.blades[lastBladeSelection].Cree2 = settings.star2Color->GetValue();
        config.blades[lastBladeSelection].Cree3 = settings.star3Color->GetValue();
        config.blades[lastBladeSelection].Cree4 = settings.star4Color->GetValue();

        config.features.bladeDetect = settings.bladeDetect->GetValue();
        config.features.bladeDetectPin = settings.bladeDetectPin->GetValue();

        if (lastSubBladeSelection != -1 && lastSubBladeSelection < (int)config.blades[lastBladeSelection].subBlades.size()) {
            config.blades[lastBladeSelection].subBlades[lastSubBladeSelection].startPixel = settings.subBladeStart->GetValue();
            config.blades[lastBladeSelection].subBlades[lastSubBladeSelection].endPixel = settings.subBladeEnd->GetValue();
        }
        config.blades[lastBladeSelection].subBladeWithStride = settings.subBladeUseStride->GetValue();
    }

    // Check if SubBlades need to be removed (changed from Neopixel)
    if (BD_HASSELECTION && lastBladeSelection == settings.bladeSelect->GetSelection() && !BD_ISNEOPIXEL) {
        config.blades[lastBladeSelection].isSubBlade = false;
        config.blades[lastBladeSelection].subBlades.clear();
    }

    // Set Values for next Run (referenced here)
    lastBladeSelection = settings.bladeSelect->GetSelection();
    lastSubBladeSelection = settings.subBladeSelect->GetSelection();


    // Rebuild/Populate Blades
    settings.bladeSelect->Clear();
    for (int bladeNum = 0; bladeNum < (int)config.blades.size(); bladeNum++) {
        settings.bladeSelect->Append(wxString::FromUTF8("Blade " + std::to_string(bladeNum)));
    }
    if ((int)settings.bladeSelect->GetCount() - 1 >= lastBladeSelection) settings.bladeSelect->SetSelection(lastBladeSelection);

    // Rebuild/Populate SubBlades
    settings.subBladeSelect->Clear();
    for (int subBladeNum = 0; lastBladeSelection != -1 && subBladeNum < (int)config.blades[lastBladeSelection].subBlades.size(); subBladeNum++) {
        settings.subBladeSelect->Append(wxString::FromUTF8("SubBlade " + std::to_string(subBladeNum)));
    }
    if ((int)settings.subBladeSelect->GetCount() - 1 >= lastSubBladeSelection) settings.subBladeSelect->SetSelection(lastSubBladeSelection);

    // Recall Options
    if (config.blades.size() > 0 && lastBladeSelection >= 0 && lastBladeSelection < (int)config.blades.size()) {
        settings.bladeType->SetValue(config.blades[lastBladeSelection].type);
        settings.usePowerPin1->SetValue(config.blades[lastBladeSelection].usePowerPin1);
        settings.usePowerPin2->SetValue(config.blades[lastBladeSelection].usePowerPin2);
        settings.usePowerPin3->SetValue(config.blades[lastBladeSelection].usePowerPin3);
        settings.usePowerPin4->SetValue(config.blades[lastBladeSelection].usePowerPin4);
        settings.usePowerPin5->SetValue(config.blades[lastBladeSelection].usePowerPin5);
        settings.usePowerPin6->SetValue(config.blades[lastBladeSelection].usePowerPin6);

        settings.bladeDataPin->SetValue(config.blades[lastBladeSelection].dataPin);
        settings.bladePixels->SetValue(config.blades[lastBladeSelection].numPixels);
        settings.blade3ColorOrder->SetValue(config.blades[lastBladeSelection].colorType);
        settings.blade4ColorOrder->SetValue(config.blades[lastBladeSelection].colorType);
        settings.blade4UseRGB->SetValue(config.blades[lastBladeSelection].useRGBWithWhite);

        settings.star1Color->SetValue(config.blades[lastBladeSelection].Cree1);
        settings.star2Color->SetValue(config.blades[lastBladeSelection].Cree2);
        settings.star3Color->SetValue(config.blades[lastBladeSelection].Cree3);
        settings.star4Color->SetValue(config.blades[lastBladeSelection].Cree4);

        settings.bladeDetect->SetValue(config.features.bladeDetect);
        settings.bladeDetectPin->SetValue(config.features.bladeDetectPin);

        settings.subBladeStart->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int)config.blades[lastBladeSelection].subBlades.size() ? config.blades[lastBladeSelection].subBlades[lastSubBladeSelection].startPixel : 0);
        settings.subBladeEnd->SetValue(lastSubBladeSelection != -1 && lastSubBladeSelection < (int)config.blades[lastBladeSelection].subBlades.size() ? config.blades[lastBladeSelection].subBlades[lastSubBladeSelection].endPixel : 0);
        settings.subBladeUseStride->SetValue(config.blades[lastBladeSelection].subBladeWithStride);
    }

    // Enable/Disable Elements
    settings.removeBlade->Enable(settings.bladeSelect->GetCount() > 0 && BD_HASSELECTION);
    settings.removeSubBlade->Enable(settings.subBladeSelect->GetCount() > 0 && BD_SUBHASSELECTION);
    settings.addSubBlade->Enable(BD_ISNEOPIXEL && BD_HASSELECTION);

    settings.bladeType->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin1->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin2->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin3->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin4->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin5->Enable(BD_HASSELECTION && BD_ISFIRST);
    settings.usePowerPin6->Enable(BD_HASSELECTION && BD_ISFIRST);

    // Show/Unshow Elements
    settings.bladeColorOrderLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.blade3ColorOrder->Show(BD_ISNEOPIXEL3 && BD_ISFIRST);
    settings.blade4ColorOrder->Show(BD_ISNEOPIXEL4 && BD_ISFIRST);
    settings.blade4UseRGB->Show(BD_ISNEOPIXEL4 && BD_ISFIRST);

    settings.bladeDataPinLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladeDataPin->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladePixelsLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladePixels->Show(BD_ISNEOPIXEL && BD_ISFIRST);

    settings.star1ColorLabel->Show(BD_ISCREE);
    settings.star1Color->Show(BD_ISCREE);
    settings.star2ColorLabel->Show(BD_ISCREE);
    settings.star2Color->Show(BD_ISCREE);
    settings.star3ColorLabel->Show(BD_ISCREE);
    settings.star3Color->Show(BD_ISCREE);
    settings.star4ColorLabel->Show(BD_ISCREE4);
    settings.star4Color->Show(BD_ISCREE4);

    settings.bladeDetect->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladeDetectPin->Show(BD_ISNEOPIXEL && BD_ISFIRST);
    settings.bladeDetectPinLabel->Show(BD_ISNEOPIXEL && BD_ISFIRST);

    settings.subBladeUseStride->Show(BD_ISSUB && BD_ISFIRST);
    settings.subBladeStartLabel->Show(BD_SUBHASSELECTION);
    settings.subBladeStart->Show(BD_SUBHASSELECTION);
    settings.subBladeEndLabel->Show(BD_SUBHASSELECTION);
    settings.subBladeEnd->Show(BD_SUBHASSELECTION);
}

MainWindow::numEntry MainWindow::createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int ID, int minVal, int maxVal, int defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrl *numEntry = new wxSpinCtrl(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal);
    numEntryBox->Add(text, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 10));
    numEntryBox->Add(numEntry);

    MainWindow::numEntry returnVal;
    returnVal.box = numEntryBox;
    returnVal.num = numEntry;

    return returnVal;
}

MainWindow::numEntry MainWindow::createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int ID, double minVal, double maxVal, double defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrlDouble *numEntry = new wxSpinCtrlDouble(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal, 0.1);
    numEntryBox->Add(text, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 10));
    numEntryBox->Add(numEntry);

    MainWindow::numEntry returnVal;
    returnVal.box = numEntryBox;
    returnVal.doubleNum = numEntry;

    return returnVal;
}

void MainWindow::outputConfig() {
    updateGeneralPage();
    updatePresetsPage();
    updateBladesPage();

    std::ofstream configOutput("./proffieConfig.h");

    // CONFIG_TOP
    {
        configOutput << "#ifdef CONFIG_TOP" << std::endl;
        switch (config.boardConfig.board) {
        case PROFFIEBOARD::V1:
            configOutput << "#include \"proffieboard_v1_config.h\"" << std::endl;
            break;
        case PROFFIEBOARD::V2:
            configOutput << "#include \"proffieboard_v2_config.h\"" << std::endl;
            break;
        case PROFFIEBOARD::V3:
            configOutput << "#include \"proffieboard_v3_config.h\"" << std::endl;
        }
        configOutput << "#define NUM_BLADES " << config.blades.size() << std::endl;
        configOutput << "#define NUM_BUTTONS " << config.general.numButtons << std::endl;
        configOutput << "#define VOLUME " << config.general.volume << std::endl;
        configOutput << "const unsigned int maxLedsPerStrip = " << config.general.maxLEDs << std::endl;
        configOutput << "#define CLASH_THRESHOLD_G " << config.general.clashThreshold << std::endl;
        // Implement Blade Detect Config
        configOutput << "#define ENABLE_AUDIO" << std::endl;
        configOutput << "#define ENABLE_WS2811" << std::endl;
        configOutput << "#define ENABLE_SD" << std::endl;
        configOutput << "#define ENABLE_MOTION" << std::endl;
        configOutput << "#define SHARED_POWER_PINS" << std::endl;
        if (config.features.hasOLED) configOutput << "#define ENABLE_SSD1306" << std::endl;
        if (config.options.saveColorChange) configOutput << "#define SAVE_COLOR_CHANGE" << std::endl;
        if (config.options.savePreset) configOutput << "#define SAVE_PRESET" << std::endl;
        if (config.options.saveVolume) configOutput << "#define SAVE_VOLUME" << std::endl;
        if (config.tweaks.devCommands) configOutput << "#define ENABLE_DEVELOPER_COMMANDS" << std::endl;
        else configOutput << "#define DISABLE_DIAGNOSTIC_COMMANDS" << std::endl;
        configOutput << "#define DISABLE_BASIC_PARSER_STYLES" << std::endl;
        configOutput << "#define PLI_OFF_TIME 60 * 1000 * " << config.tweaks.pliTimeout << std::endl;
        configOutput << "#define IDLE_OFF_TIME 60 * 1000 * " << config.tweaks.idleTimout << std::endl;
        configOutput << "#define MOTION_TIMEOUT 60 * 1000 * " << config.tweaks.motionTimeout << std::endl;
        if (config.propConfig.prop == SABERPROP::SA22C) {
            if (config.propConfig.revertLockup) configOutput << "#define SA22C_NO_LOCKUP_HOLD" << std::endl;
            if (config.propConfig.stabOn) configOutput << "#define SA22C_STAB_ON" << std::endl;
            if (config.propConfig.swingOn) configOutput << "#define SA22C_SWING_ON" << std::endl;
            if (config.propConfig.twistOn) configOutput << "#define SA22C_TWIST_ON" << std::endl;
            if (config.propConfig.thrustOn) configOutput << "#define SA22C_THRUST_ON" << std::endl;
            if (config.propConfig.twistOff) configOutput << "#define SA22C_TWIST_OFF" << std::endl;
            if (config.propConfig.forcePush) {
                configOutput << "#define SA22C_FORCE_PUSH" << std::endl;
                configOutput << "#define SA22C_FORCE_PUSH_LENGTH << " << config.propConfig.forcePushLength << std::endl;
            }
            if (config.propConfig.gestureBattle) configOutput << "#define GESTURE_AUTO_BATTLE_MODE" << std::endl;
            configOutput << "#define SA22C_LOCKUP_DELAY " << config.propConfig.lockupDelay << std::endl;
        } // Add other props and options
        configOutput << "#endif" << std::endl << std::endl; // CONFIG_TOP
    }
    // CONFIG_PROP
    {
        configOutput << "#ifdef CONFIG_PROP" << std::endl;
        if (config.propConfig.prop == SABERPROP::SA22C) configOutput << "#include \"../props/saber_sa22c_buttons.h\"" << std::endl;
        configOutput << "#endif" << std:: endl << std::endl; // CONFIG_PROP
    }
    // CONFIG_PRESETS
    {
        configOutput << "#ifdef CONFIG_PRESETS" << std::endl;
        configOutput << "Preset blade_in[] = {" << std::endl;
        for (presetConfig preset : config.presets) {
            configOutput << "\t{ \"" << preset.dirs << "\", \"" << preset.track << "\"," << std::endl;
            if (preset.styles.size() > 0) for (std::string style : preset.styles) configOutput << "\t\t" << style << "," << std::endl;
            else configOutput << "\t\t," << std::endl;
            configOutput << "\t\t\"" << preset.name << "\"}";
            // If not the last one, add comma
            if (&config.presets[config.presets.size() - 1] != &preset) configOutput << ",";
            configOutput << std::endl;
        }
        configOutput << "};" << std::endl;

        // Configure Blades
        auto genWS281X = [&](bladeConfig blade) {
            std::string bladePin = blade.dataPin == "Pin 1" ? "bladePin" : blade.dataPin == "Pin 2" ? "blade2Pin" : blade.dataPin == "Pin 3" ? "blade3Pin" : "blade4Pin";
            std::string bladeColor = blade.type == "NeoPixel (RGB)" || blade.useRGBWithWhite ? blade.colorType : [=](std::string colorType) -> std::string { colorType.replace(colorType.find("W"), 1, "w"); return colorType; }(blade.colorType);

            configOutput << "WS281XBladePtr<" << blade.numPixels << ", " << bladePin << ", Color8::" << bladeColor << ", PowerPINS<";
            if (blade.usePowerPin1) {
                configOutput << "bladePowerPin1";
                if (blade.usePowerPin2 || blade.usePowerPin3 || blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
            }
            if (blade.usePowerPin2) {
                configOutput << "bladePowerPin2";
                if (blade.usePowerPin3 || blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
            }
            if (blade.usePowerPin3) {
                configOutput << "bladePowerPin3";
                if (blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
            }
            if (blade.usePowerPin4) {
                configOutput << "bladePowerPin4";
                if (blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
            }
            if (blade.usePowerPin5) {
                configOutput << "bladePowerPin5";
                if (blade.usePowerPin6) configOutput << ", ";
            }
            if (blade.usePowerPin6) {
                configOutput << "bladePowerPin6";
            }
            configOutput << ">>()";
        };
        configOutput << "BladeConfig blades[] = {" << std::endl;
        configOutput << "\t{ 0," << std::endl;
        for (bladeConfig blade : config.blades) {
            if (blade.type == "NeoPixel (RGB)" || blade.type == "NeoPixel (RGBW)") {
                bool firstSub = true;
                if (blade.isSubBlade) for (bladeConfig::subBladeInfo subBlade : blade.subBlades) {
                    configOutput << "\t\tSubBlade( " << subBlade.startPixel << ", " << subBlade.endPixel << ", ";
                    if (firstSub) {
                        genWS281X(blade);
                        configOutput << ")," << std::endl;
                    } else {
                        configOutput << "NULL)," << std::endl;
                    }
                    firstSub = false;
                } else {
                    configOutput << "\t\t";
                    genWS281X(blade);
                    configOutput << "," << std::endl;
                }
            } else if (blade.type == "Tri-Star Cree" || blade.type == "Quad-Star Cree") {
                bool powerPins[4]{true, true, true, true};
                configOutput << "\t\tSimpleBladePtr<";
                if (blade.Cree1 != "<None>") configOutput << "CreeXPE2" << blade.Cree1 << "Template<" << blade.Cree1Resistance << ">, ";
                else {
                    configOutput << "NoLED, ";
                    powerPins[0] = false;
                }
                if (blade.Cree2 != "<None>") configOutput << "CreeXPE2" << blade.Cree2 << "Template<" << blade.Cree2Resistance << ">, ";
                else {
                    configOutput << "NoLED, ";
                    powerPins[1] = false;
                }
                if (blade.Cree3 != "<None>") configOutput << "CreeXPE2" << blade.Cree3 << "Template<" << blade.Cree3Resistance << ">, ";
                else {
                    configOutput << "NoLED, ";
                    powerPins[2] = false;
                }
                if (blade.Cree4 != "<None>" && blade.type != "Tri-Star Cree") configOutput << "CreeXPE2" << blade.Cree4 << "Template<" << blade.Cree4Resistance << ">, ";
                else {
                    configOutput << "NoLED, ";
                    powerPins[3] = false;
                }

                bladeConfig tempBlade = blade;
                for (int powerPin = 0; powerPin < 4; powerPin++) {
                    if (powerPins[powerPin]) {
                        if (tempBlade.usePowerPin1) {
                            configOutput << "bladePowerPin1";
                            tempBlade.usePowerPin1 = false;
                        } else if (tempBlade.usePowerPin2) {
                            configOutput << "bladePowerPin2";
                            tempBlade.usePowerPin2 = false;
                        } else if (tempBlade.usePowerPin3) {
                            configOutput << "bladePowerPin3";
                            tempBlade.usePowerPin3 = false;
                        } else if (tempBlade.usePowerPin4) {
                            configOutput << "bladePowerPin4";
                            tempBlade.usePowerPin4 = false;
                        } else if (tempBlade.usePowerPin5) {
                            configOutput << "bladePowerPin5";
                            tempBlade.usePowerPin5 = false;
                        } else if (tempBlade.usePowerPin6) {
                            configOutput << "bladePowerPin6";
                            tempBlade.usePowerPin6 = false;
                        } else configOutput << "-1";
                    } else {
                        configOutput << "-1";
                    }

                    if (powerPin != 3) configOutput << ", ";
                }
                configOutput << ">()," << std::endl;
            } else if (blade.type == "Single Color") {
                configOutput << "\t\tSimpleBladePtr<CreeXPE2WhiteTemplate<550>, NoLED, NoLED, NoLED, ";
                if (blade.usePowerPin1) {
                    configOutput << "bladePowerPin1, ";
                } else if (blade.usePowerPin2) {
                    configOutput << "bladePowerPin2, ";
                } else if (blade.usePowerPin3) {
                    configOutput << "bladePowerPin3, ";
                } else if (blade.usePowerPin4) {
                    configOutput << "bladePowerPin4, ";
                } else if (blade.usePowerPin5) {
                    configOutput << "bladePowerPin5, ";
                } else if (blade.usePowerPin6) {
                    configOutput << "bladePowerPin6, ";
                } else configOutput << "-1, ";
                configOutput << "-1, -1, -1>()," << std::endl;
            }
        }
        configOutput << "\t\tCONFIGARRAY(blade_in)" << std::endl << "\t}" << std::endl << "};" << std::endl;
        configOutput << "#endif" << std::endl << std::endl;
    }
    // CONFIG_BUTTONS
    {
        configOutput << "#ifdef CONFIG_BUTTONS" << std::endl;
        configOutput << "Button PowerButton(BUTTON_POWER, powerButtonPin, \"pow\");" << std::endl;
        if (config.general.numButtons >= 2) configOutput << "Button AuxButton(BUTTON_AUX, auxPin, \"aux\");" << std::endl;
        if (config.general.numButtons == 3) configOutput << ""; // figure out aux2 syntax
        configOutput << "#endif" << std::endl << std::endl; // CONFIG_BUTTONS
    }

    configOutput.close();
}

MainWindow::CREETYPE MainWindow::stringToCreeType(std::string input) {
    if (input == "Red") {
        return CREETYPE::RED;
    } else if (input == "Green") {
        return CREETYPE::GREEN;
    } else if (input == "Blue") {
        return CREETYPE::BLUE;
    } else if (input == "Amber") {
        return CREETYPE::AMBER;
    } else if (input == "RedOrange") {
        return CREETYPE::REDORANGE;
    } else if (input == "White") {
        return CREETYPE::WHITE;
    } else {
        return CREETYPE::NOLED;
    }
}

std::string MainWindow::creeTypeToString(CREETYPE input) {
    switch (input) {
    case CREETYPE::RED:
        return "Red";
    case CREETYPE::GREEN:
        return "Green";
    case CREETYPE::BLUE:
        return "Blue";
    case CREETYPE::AMBER:
        return "Amber";
    case CREETYPE::REDORANGE:
        return "RedOrange";
    case CREETYPE::WHITE:
        return "White";
    case CREETYPE::NOLED:
        return "<None>";
    default:
        return "<None>";
    }
}

MainWindow::C_ORDER MainWindow::stringToColorOrder(std::string input, bool useRGB) {
    if (input == "BGR") {
        return C_ORDER::BGR;
    } else if (input == "BRG") {
        return C_ORDER::BRG;
    } else if (input == "GBR") {
        return C_ORDER::GBR;
    } else if (input == "GRB") {
        return C_ORDER::GRB;
    } else if (input == "RBG") {
        return C_ORDER::RBG;
    } else if (input == "RGB") {
        return C_ORDER::RGB;
    } else if (input == "BGRW") {
        return useRGB ? C_ORDER::BGRW : C_ORDER::BGRw;
    } else if (input == "BRGW") {
        return useRGB ? C_ORDER::BRGW : C_ORDER::BRGw;
    } else if (input == "GBRW") {
        return useRGB ? C_ORDER::GBRW : C_ORDER::GBRw;
    } else if (input == "GRBW") {
        return useRGB ? C_ORDER::GRBW : C_ORDER::GRBw;
    } else if (input == "RGBW") {
        return useRGB ? C_ORDER::RBGW : C_ORDER::RGBw;
    } else if (input == "RGBW") {
        return useRGB ? C_ORDER::RGBW : C_ORDER::RGBw;
    } else if (input == "WBGR") {
        return useRGB ? C_ORDER::WBGR : C_ORDER::wBGR;
    } else if (input == "WBRG") {
        return useRGB ? C_ORDER::WBRG : C_ORDER::wBRG;
    } else if (input == "WGBR") {
        return useRGB ? C_ORDER::WGBR : C_ORDER::wGBR;
    } else if (input == "WGRB") {
        return useRGB ? C_ORDER::WGRB : C_ORDER::wGRB;
    } else if (input == "WRBG") {
        return useRGB ? C_ORDER::WRBG : C_ORDER::wRBG;
    } else if (input == "WRGB") {
        return useRGB ? C_ORDER::WRGB : C_ORDER::wRGB;
    } else {
        return C_ORDER::GRB;
    }
}

std::string MainWindow::colorOrder3ToString(C_ORDER input) {
    switch (input) {
    case C_ORDER::GRB:
        return "GRB";
    case C_ORDER::GBR:
        return "GBR";
    case C_ORDER::RGB:
        return "RGB";
    case C_ORDER::RBG:
        return "RBG";
    case C_ORDER::BGR:
        return "BGR";
    case C_ORDER::BRG:
        return "BRG";
    default:
        return "GRB";
    }
}

std::string MainWindow::colorOrder4ToString(C_ORDER input) {
    switch (input) {
    case C_ORDER::GRBW:
    case C_ORDER::GRBw:
        return "GRBW";
    case C_ORDER::GBRW:
    case C_ORDER::GBRw:
        return "GBRW";
    case C_ORDER::RGBW:
    case C_ORDER::RGBw:
        return "RGBW";
    case C_ORDER::RBGW:
    case C_ORDER::RBGw:
        return "RBGW";
    case C_ORDER::BRGW:
    case C_ORDER::BRGw:
        return "BRGW";
    case C_ORDER::BGRW:
    case C_ORDER::BGRw:
        return "BGRW";
    case C_ORDER::WGRB:
    case C_ORDER::wGRB:
        return "WGRB";
    case C_ORDER::WGBR:
    case C_ORDER::wGBR:
        return "WGBR";
    case C_ORDER::WRGB:
    case C_ORDER::wRGB:
        return "WRGB";
    case C_ORDER::WRBG:
    case C_ORDER::wRBG:
        return "WRBG";
    case C_ORDER::WBRG:
    case C_ORDER::wBRG:
        return "WBRG";
    case C_ORDER::WBGR:
    case C_ORDER::wBGR:
        return "WBGR";
    default:
        return "GRBW";
    }
}

bool MainWindow::colorOrderUseRGB(C_ORDER input) {
    switch (input) {
    case C_ORDER::BGRW:
    case C_ORDER::BRGW:
    case C_ORDER::GRBW:
    case C_ORDER::GBRW:
    case C_ORDER::RGBW:
    case C_ORDER::RBGW:
        return true;
    default:
        return false;
    }
}

MainWindow::BLADETYPE MainWindow::stringToBladeType(std::string input) {
    if (input == "NeoPixel (RGB)") {
        return BLADETYPE::NEOPIXEL_3;
    } else if (input == "NeoPixel (RGBW)") {
        return BLADETYPE::NEOPIXEL_4;
    } else if (input == "Tri-Star Cree") {
        return BLADETYPE::CREE_3;
    } else if (input == "Quad-Star Cree") {
        return BLADETYPE::CREE_4;
    } else if (input == "Single Color") {
        return BLADETYPE::SINGLECOLOR;
    } else {
        return BLADETYPE::SINGLECOLOR;
    }
}

std::string MainWindow::bladeTypeToString(BLADETYPE input) {
    switch (input) {
    case BLADETYPE::NEOPIXEL_3:
        return "NeoPixel (RGB)";
    case BLADETYPE::NEOPIXEL_4:
        return "NeoPixel (RGBW)";
    case BLADETYPE::CREE_3:
        return "Tri-Star Cree";
    case BLADETYPE::CREE_4:
        return "Quad-Star Cree";
    case BLADETYPE::SINGLECOLOR:
        return "Single Color";
    default:
        return "<None>";
    }
}

int MainWindow::stringToDataPin(std::string input) {
    if (input == "Pin 1") return 1;
    if (input == "Pin 2") return 2;
    if (input == "Pin 3") return 3;
    if (input == "Pin 4") return 4;
    return -1;
}

std::string MainWindow::dataPinToString(int input) {
    if (input > 0) {
        return "Pin " + std::to_string(input);
    }
    else return "";
}

void MainWindow::OnExit(wxCommandEvent&) { Close(true); }

void MainWindow::OnAbout(wxCommandEvent&) { wxMessageBox("Tool for GUI configuration and flashing of ProffieBoard\n\nCreated by Ryryog25", "About ProffieConfig", wxOK | wxICON_INFORMATION); }

wxIMPLEMENT_APP(ProffieConfig);

bool ProffieConfig::OnInit() {
    MainWindow *frame = new MainWindow();
    frame->Show(true);
    return true;
}
