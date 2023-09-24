#include "generalpage.h"
#include "defines.h"
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

GeneralPage::GeneralPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "General")
{
    wxBoxSizer *generalHoriz = new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *boardSetup = new wxStaticBoxSizer(wxVERTICAL, GetStaticBox(), "Board Setup");
    wxStaticBoxSizer *propSetup = new wxStaticBoxSizer(wxHORIZONTAL, GetStaticBox(), "Prop Setup");
    wxStaticBoxSizer *options = new wxStaticBoxSizer(wxHORIZONTAL, GetStaticBox(), "Options");

    wxWrapSizer *propSetup1 = new wxWrapSizer(wxVERTICAL, 0);
    wxWrapSizer *options1 = new wxWrapSizer(wxVERTICAL, 0);

    wxStaticBoxSizer *battleMode = new wxStaticBoxSizer(wxVERTICAL, propSetup->GetStaticBox(), "Battle Mode");

    // boardSetup
    {
      settings.board = new wxComboBox(boardSetup->GetStaticBox(), wxID_ANY, "ProffieBoard V2", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"ProffieBoard V1", "ProffieBoard V2", "ProffieBoard V3"}), wxCB_READONLY);
        settings.massStorage = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable Mass Storage");
        settings.webUSB = new wxCheckBox(boardSetup->GetStaticBox(), wxID_ANY, "Enable WebUSB");

        boardSetup->Add(settings.board, wxSizerFlags(0).Border(wxALL, 10));
        boardSetup->Add(settings.massStorage, wxSizerFlags(0).Border(wxALL, 10));
        boardSetup->Add(settings.webUSB, wxSizerFlags(0).Border(wxALL, 10));
    }

    // propSetup
    {
        // General
        settings.prop = new wxComboBox(propSetup->GetStaticBox(), wxID_ANY, PR_SA22C, wxDefaultPosition, wxDefaultSize, Misc::createEntries({PR_DEFAULT, PR_SA22C, PR_FETT263, PR_SHTOK, PR_BC}), wxCB_READONLY);
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

        settings.lockupDelay = Misc::createNumEntry(battleMode, "Lockup Delay (ms)", wxID_ANY, 0, 3000, 200);

        // force push
        settings.forcePush = new wxCheckBox(propSetup->GetStaticBox(), wxID_ANY, "Enable Force Push");
        settings.forcePushLength = Misc::createNumEntry(propSetup, "Force Push Length", wxID_ANY, 0, 10, 5);

        // Prop Setup 1
        propSetup1->Add(settings.prop, MENUITEMFLAGS);
        propSetup1->Add(settings.stabOn, MENUITEMFLAGS);
        propSetup1->Add(settings.swingOn, MENUITEMFLAGS);
        propSetup1->Add(settings.twistOn, MENUITEMFLAGS);
        propSetup1->Add(settings.thrustOn, MENUITEMFLAGS);
        // Prop Setup 2
        //propSetup2->Add(settings.battleEnable, MENUITEMFLAGS);
        propSetup1->Add(settings.gestureEnBattle, MENUITEMFLAGS);
        propSetup1->Add(settings.noLockupHold, MENUITEMFLAGS);
        propSetup1->Add(settings.twistOff, MENUITEMFLAGS);
        propSetup1->Add(settings.forcePush, MENUITEMFLAGS);
        propSetup1->Add(settings.forcePushLength.box, MENUITEMFLAGS);
        // Battle Mode
        battleMode->Add(settings.lockupDelay.box, wxSizerFlags(0).Border(wxALL, 10));
        propSetup1->Add(battleMode, wxSizerFlags(0).Border(wxALL, 10));

        propSetup->Add(propSetup1, wxSizerFlags(1));
    }

    // Options
    {
        settings.buttons = Misc::createNumEntry(options, "Number of Buttons", wxID_ANY, 1, 3, 2);
        settings.volume = Misc::createNumEntry(options, "Max Volume", wxID_ANY, 0, 3500, 2000);
        settings.clash = Misc::createNumEntryDouble(options, "Clash Threshold", wxID_ANY, 0.1, 5, 3);
        settings.pliTime = Misc::createNumEntry(options, "PLI Timeout", wxID_ANY, 1, 60, 2);
        settings.idleTime = Misc::createNumEntry(options, "Idle Timeout", wxID_ANY, 1, 60, 10);
        settings.motion = Misc::createNumEntry(options, "Motion Timeout", wxID_ANY, 1, 60, 15);

        settings.volumeSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Volume");
        settings.presetSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Preset");
        settings.colorSave = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Save Color");
        settings.disableColor = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Color Change");
        settings.disableDev = new wxCheckBox(options->GetStaticBox(), wxID_ANY, "Disable Developer Commands");

        // Options 1
        options1->Add(settings.buttons.box, MENUITEMFLAGS);
        options1->Add(settings.volume.box, MENUITEMFLAGS);
        options1->Add(settings.clash.box, MENUITEMFLAGS);
        options1->Add(settings.pliTime.box, MENUITEMFLAGS);
        options1->Add(settings.idleTime.box, MENUITEMFLAGS);
        options1->Add(settings.motion.box, MENUITEMFLAGS);
        options1->Add(settings.volumeSave, MENUITEMFLAGS);
        options1->Add(settings.presetSave, MENUITEMFLAGS);
        options1->Add(settings.colorSave, MENUITEMFLAGS);
        options1->Add(settings.disableColor, MENUITEMFLAGS);
        options1->Add(settings.disableDev, MENUITEMFLAGS);

        options->Add(options1);
    }

    generalHoriz->Add(boardSetup, wxSizerFlags(/*proportion*/ 2).Border(wxALL, 10).Expand());
    generalHoriz->Add(propSetup, wxSizerFlags(/*proportion*/ 7).Border(wxALL, 10).Expand());

    Add(generalHoriz, wxSizerFlags(10).Expand());
    Add(options, wxSizerFlags(8).Border(wxALL, 10).Expand());
}

decltype(GeneralPage::settings) GeneralPage::settings;
