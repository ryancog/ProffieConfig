// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "pages/proppage.h"

#include "core/appstate.h"
#include "elements/misc.h"
#include "core/defines.h"
#include "pages/generalpage.h"
#include "core/mainwindow.h"
#include "core/propfile.h"
#include "config/configuration.h"

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

PropPage* PropPage::instance;
PropPage::PropPage(wxWindow* window) : wxScrolledWindow(window) {
  PropPage::instance = this;

  sizer = new wxStaticBoxSizer(wxVERTICAL, this, "");
  auto top = new wxBoxSizer(wxHORIZONTAL);
  propSelection = new wxComboBox(sizer->GetStaticBox(), ID_Select, PR_DEFAULT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Default"}), wxCB_READONLY);
  buttonInfo = new wxButton(sizer->GetStaticBox(), ID_Buttons, "Buttons...");
  top->Add(propSelection, BOXITEMFLAGS);
  top->Add(buttonInfo, BOXITEMFLAGS);

  sizer->Add(top);

  AppState::instance->clearProps();
  for (const auto& prop : AppState::instance->getPropFileNames()) {
    AppState::instance->addProp(PropFile(prop));
  }
  updateProps();

  bindEvents();

  SetSizerAndFit(sizer);
  SetScrollbars(-1, 10, -1, 1);
}

void PropPage::bindEvents() {
  auto propSelectUpdate = [&](wxCommandEvent&) {
    PropPage::updatePropSelection();
    PropPage::instance->SetMinClientSize(wxSize(PropPage::instance->sizer->GetMinSize().GetWidth(), 0));
    FULLUPDATEWINDOW;
    MainWindow::instance->SetSize(wxSize(MainWindow::instance->GetSize().GetWidth(), MainWindow::instance->GetMinHeight() + PropPage::instance->GetBestVirtualSize().GetHeight()));
    MainWindow::instance->SetMinSize(wxSize(MainWindow::instance->GetSize().GetWidth(), 350));
  };
  auto optionSelectUpdate = [](wxCommandEvent&) {
    int32_t x, y;
    PropPage::instance->GetViewStart(&x, &y);
    PropPage::instance->update();
    PropPage::instance->Scroll(0, y);
  };

  Bind(wxEVT_COMBOBOX, propSelectUpdate, ID_Select);
  Bind(wxEVT_CHECKBOX, optionSelectUpdate, wxID_ANY);
  Bind(wxEVT_RADIOBUTTON, optionSelectUpdate, wxID_ANY);
  Bind(wxEVT_SPINCTRL, optionSelectUpdate, wxID_ANY);
  Bind(wxEVT_SPINCTRLDOUBLE, optionSelectUpdate, wxID_ANY);

  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxString buttons;
        switch (Configuration::instance->parsePropSel(propSelection->GetValue().ToStdString())) {
          case Configuration::SaberProp::DEFAULT:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() == 0 ? wxString (
                     "On/Off - Twist\n"
                     "Next preset - Point up and shake\n"
                     "Clash - Hit the blade while saber is on."
                     ) : GeneralPage::instance->buttons->num->GetValue() == 1 ? wxString(
                       "On/Off - Click to turn the saber on or off.\n"
                       "Turn On muted - Double-click\n"
                       "Next preset - Hold button and hit the blade while saber is off.\n"
                       "Clash - Hit the blade while saber is on.\n"
                       "Lockup - Hold button, then trigger a clash. Release button to end.\n"
                       "Drag - Hold button, then trigger a clash while pointing down. Release button to end.\n"
                       "Melt - Hold button and stab something.\n"
                       "Force - Long-click button.\n"
                       "Start Soundtrack - Long-click the button while blade is off.\n"
                       "Enter/Exit Color Change - Hold button and Twist."
                       ) : GeneralPage::instance->buttons->num->GetValue() == 2 || GeneralPage::instance->buttons->num->GetValue() == 3 ? wxString (
                       "On/Off - Click POW\n"
                       "Turn On muted - Double-click POW button\n"
                       "Next preset - Hold POW button and hit the blade while saber is off.\n"
                       "Previous Preset - Hold AUX button and click the POW button while saber is off.\n"
                       "Clash - Hit the blade while saber is on.\n"
                       "Lockup -  Hold either POW or AUX, then trigger a clash. Release button to end.\n"
                       "Drag - Hold either POW or AUX, then trigger a clash while pointing down. Release button to end.\n"
                       "Melt - Hold either POW or AUX and stab something.\n"
                       "Force Lightning Block - Click AUX while holding POW.\n"
                       "Force - Long-click POW button.\n"
                       "Start Soundtrack - Long-click the POW button while blade is off.\n"
                       "Blaster block - Short-click AUX button.\n"
                       "Enter/Exit Color Change - Hold Aux and click POW while on."
                       ) : wxString("Button Configuration Not Supported"));
            break;
          case Configuration::SaberProp::SA22C:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() == 1 ? wxString(
                     "Activate Muted - double click and hold while OFF\n"
                     "Activate - short click while OFF\n"
                     "Play/Stop Music - double click while OFF\n"
                     "Turn off blade - hold and wait till blade is off while ON\n"
                     "Next Preset - hold and release while OFF\n"
                     "Prev Preset - hold and wait while OFF\n"
                     "Lockup - hold + hit clash while ON\n"
                     "Stab - thrust forward clash while ON\n"
                     "Lightning Block - double click and hold while ON\n"
                     "Melt - hold + thust forward clash while ON\n"
                     "Drag - hold + hit clash while ON pointing the blade tip down\n"
                     "Blaster Blocks - short click/double click/triple click while on\n"
                     "Multi-Blast - hold while swinging for one second and release\n"
                     "              to trigger blaster block, swing saber while in multi-blast mode\n"
                     "              to exit, hold while swinging for one second and release\n"
                     "Battle Mode - triple-click and hold while on\n"
                     "Force Effects - hold + twist the hilt while ON (while pointing up)\n"
                     "Color Change mode - hold + twist the hilt while ON (pointing down)\n"
                     "Enter Volume - Menu hold + clash while OFF\n"
                     "Volume UP - hold and release while in Volume Menu\n"
                     "Volume DOWN - click while in Volume Menu\n"
                     "Exit Volume Menu - Menu hold + clash while OFF\n"
                     "Battery Level - triple click while OFF")
                 : GeneralPage::instance->buttons->num->GetValue() == 2 ? wxString(
                       "POWER:\n"
                       "    Activate Muted - double click and hold while OFF\n"
                       "    Activate - short click while OFF\n"
                       "    Play/Stop Music - hold and release while OFF\n"
                       "    Turn off blade - hold and wait till blade is off while ON\n"
                       "    Force Effects - double click while ON\n"
                       "    Volume UP - short click while OFF and in VOLUME MENU\n"
                       "    Prev Preset - hold and wait while OFF\n"
                       "    Color Change mode - hold + toggle AUX while ON\n"
                       "    Lightning Block - double click and hold while ON\n"
                       "    Melt - hold while stabbing (clash with forward motion, horizontal)\n"
                       "    Battle Mode - triple-click and hold for half a second while on\n"
                       "AUX:\n"
                       "    Blaster blocks - short click/double click/triple click while ON\n"
                       "    Multi-Blast - double-click and hold for half a second\n"
                       "                  to trigger blaster block, swing saber while in multi-blast mode\n"
                       "                  to exit, double-click and hold for half a second\n"
                       "    Next Preset - short click while OFF\n"
                       "    Lockup - hold while ON\n"
                       "    Drag - hold while ON pointing the blade tip down\n"
                       "    Enter VOLUME MENU - long click while OFF\n"
                       "    Volume down - short click while OFF and in VOLUME MENU\n"
                       "    Battery level - hold while off"
                       )
                 : GeneralPage::instance->buttons->num->GetValue() ? wxString(
                       "POWER:\n"
                       "    Activate Muted - double click and hold while OFF\n"
                       "    Activate - short click while OFF\n"
                       "    Play/Stop Music - hold and release while OFF\n"
                       "    Turn off blade - hold and wait till blade is off while ON\n"
                       "    Force Effects - double click while ON\n"
                       "    Volume UP - short click while OFF and in VOLUME MENU\n"
                       "    Color Change mode - hold + toggle AUX while ON\n"
                       "    Melt - hold while stabbing (clash with forward motion, horizontal)\n"
                       "AUX:\n"
                       "    Blaster blocks - short click/double click/triple click while ON\n"
                       "    Multi-Blast - double-click and hold for half a second\n"
                       "                  to trigger blaster block, swing saber while in multi-blast mode\n"
                       "                  to exit, double-click and hold for half a second\n"
                       "    Next Preset - short click while OFF\n"
                       "    Lockup - hold while ON\n"
                       "    Drag - hold while ON pointing the blade tip down\n"
                       "    Enter VOLUME MENU - long click while OFF\n"
                       "    Volume down - short click while OFF and in VOLUME MENU\n"
                       "    Battery level - hold while off\n"
                       "AUX2:\n"
                       "    Lightning Block - hold while ON\n"
                       "    Battle Mode - double-click and hold while on\n"
                       "    Previous Preset - short click while OFF"
                       ) : wxString("Button Configuration Not Supported")) +
                "\n\n"
                "Battle mode by Fett263\n"
                "\n"
                "Once you enter battle mode, button functions will be disabled for lockup\n"
                "stab, melt, etc.  Blaster blocks and lightning block will continue to be\n"
                "triggered by button controls.  Automatic lockup/clash detection works\n"
                "by measuring delay of the saber blade pulling back from the clash.\n"
                "If you clash the blade and it does not pull back during the delay period,\n"
                "it is assumed to be a lockup and the lockup effect will show on the blade.\n"
                "If you clash the blade and pull away, only the bgn/end lockup effects will show.";
            break;
          case Configuration::SaberProp::FETT263:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() == 2 || GeneralPage::instance->buttons->num->GetValue() == 3 ? wxString(
                     "Standard Controls While Blade is OFF\n"
                     "  Turn On / Ignite Saber = Click PWR\n"
                     "  Turn On / Ignite Saber (Muted) = Double Click PWR\n"
                     "  Change Preset (one at a time) = Click AUX\n"
                     "    If pointing down will go to previous\n"
                     "  Scroll Presets (using twist menu) = Long Click AUX\n"
                     "    Turn Right (Stepped) = Next Preset\n"
                     "      Increment by 5 = Hold PWR + Turn Right\n"
                     "    Turn Left (Stepped) = Previous Preset\n"
                     "      Increment by 5 = Hold PWR + Turn Left\n"
                     "    Click PWR = Select Preset\n"
                     "    Hold PWR = Select and Ignite Preset\n"
                     "    Click AUX = go to First Preset\n"
                     "  Play Track = Long Click PWR pointing up\n"
                     "  Track Player* = Long Click PWR parallel\n"
                     "    Turn Right (Stepped) = Next Track\n"
                     "    Turn Left (Stepped) = Previous Track\n"
                     "    Click PWR = Play Current Track Once\n"
                     "    Click AUX = Random (will play current track and then randomly select next tracks)\n"
                     "    Hold PWR + Turn Right = Rotate (will play current track and then next sequential tracks)\n"
                     "    Hold PWR + Turn Left = Loop Current Track\n"
                     "    Long Click PWR = Stop Track Player\n"
                     "  Force/Quote Player = Hold PWR \n"
                     "    Pointing straight down will toggle between Force/Quote\n"
                     "    If parallel will do Force/Quote based on current mode\n"
                     "  Toggle Gesture Sleep = Hold PWR + Clash\n"
                     "    Gestures sleep automatically if Blade Detect is enabled and blade is missing\n"
                     "  Toggle Spin Mode = Hold PWR + Swing\n"
                     "    Disables Clash, Stab and Lockup effects to allow for spinning and flourishes\n"
                     "  Special Abilities (Style Controlled) \n"
                     "    Hold PWR + Turn Right = Special Ability 5 (USER5)\n"
                     "    Hold PWR + Turn Left = Special Ability 6 (USER6)\n"
                     "    Hold AUX + Turn Right = Special Ability 7 (USER7)\n"
                     "    Hold Aux + Turn Left = Special Ability 8 (USER8)\n"
                     "  Volume Menu = Hold PWR, Click AUX\n"
                     "    Turn Right (Stepped) = Increase Volume (to max)\n"
                     "    Turn Left (Stepped) = Decrease Volume (to min)\n"
                     "    Click PWR or AUX = Exit\n"
                     "  Check Battery Level  = Hold AUX, Click PWR\n"
                     "  Change Font\n"
                     "    Next Font = Hold AUX + Long Click PWR (parallel or up)\n"
                     "    Previous Font= Hold AUX + Long Click PWR (down)\n"
                     "  Copy Preset = Hold PWR + Long Click AUX\n"

                     "Standard Controls While Blade is ON\n"
                     "  Turn Off / Retract Blade = Click PWR (Hold PWR**)\n"
                     "  Turn Off / Retract Blade (PowerLock Mode) = Hold PWR + Hold AUX\n"
                     "  Blast Effect = Click Aux\n"
                     "  Multi-Blast Mode = Long Click Aux\n"
                     "    Each Swing in Multi-Blast Mode will deflect Blast effect\n"
                     "    To exit, click AUX or do Clash\n"
                     "  Clash Effect = Clash Saber\n"
                     "  Stab Effect = Stab (thrust and impact tip of blade on object)\n"
                     "  Lockup Effect = Hold PWR + Clash Saber\n"
                     "  Drag Effect = Hold AUX + Stab Down\n"
                     "  Melt Effect = Hold AUX + Stab Parallel or Up\n"
                     "  Lightning Block Effect = Hold PWR + click AUX\n"
                     "  Force/Quote = Long Click PWR (parallel or down)\n"
                     "    If quotes exist in current font pointing straight down will toggle between Force/Quote\n"
                     "    If parallel will do Force/Quote based on current mode\n"
                     "  Start/Stop Tracks = Long Click PWR (pointing straight up)\n"
                     "    Default track only (use Track Player while OFF to select tracks or playback modes)\n"
                     "  Color Change = Hold AUX + Click PWR (parallel or down)\n"
                     "    Rotate Hilt to select color\n"
                     "    Click PWR to save\n"
                     "    Click AUX to revert\n"
                     "    Color Zoom = Hold PWR, Release to Save\n"
                     "       Zoom in color for easier selection\n"
                     "       Release PWR to save\n"
                     "  Power Save = Hold AUX + Click PWR (pointing straight up)\n"
                     "  Change Style (All Blades)\n"
                     "    Next Style = Hold AUX + Long Click PWR (parallel or up)\n"
                     "    Previous Style = Hold AUX + Long Click PWR (down)\n"
                     "  Multi-Phase Preset Change\n"
                     "    Hold AUX + Twist =  Next Preset\n"
                     "    Hold PWR + Twist = Previous Preset\n"
                     "  Special Abilities (Style Controlled)\n"
                     "    Hold PWR + Turn Right = Special Ability 1 (USER1)\n"
                     "    Hold PWR + Turn Left = Special Ability 2 (USER2)\n"
                     "    Hold AUX + Turn Right = Special Ability 3 (USER3)\n"
                     "    Hold Aux + Turn Left = Special Ability 4 (USER4)\n"

                     "\"Battle Mode\" Controls - While ON\n"
                     "  Enter/Exit Battle Mode = Hold AUX\n"
                     "  Clash / Lockup = controlled by gesture\n"
                     "    Clash blade\n"
                     "      If blade swings through the clash it will do a \"glancing Clash\"\n"
                     "      If blade stops/slows on clash the saber will initiate Begin Lockup\n"
                     "      To perform a \"clash\" do an immediate Pull Away this will transition from Begin Lockup to End Lockup in quick succession\n"
                     "      To Lockup, steady the blade after Clash\n"
                     "      To end Lockup do Pull Away\n"
                     "  Drag / Melt = controlled by gesture\n"
                     "    Stab (thrust with impact at tip of blade)\n"
                     "      If pointing down Drag will initiate\n"
                     "      To end Drag pull blade up from floor at an angle\n"
                     "      If parallel or up Melt will initiate\n"
                     "      To end Melt pull blade away from object at an angle\n"
                     "  Blast Effect = Click AUX\n"
                     "    After Blast, swing within 2 seconds to enter Multi-Blast Mode\n"
                     "  Multi-Blast Mode = Long Click AUX\n"
                     "    Each Swing in Multi-Blast Mode will deflect Blast effect\n"
                     "    To exit, click AUX or do Clash\n"
                     "  Lightning Block = Hold PWR, Click AUX\n"
                     "  Force Push = Push Saber\n"
                     "  Force/Quote = Long Click PWR (parallel or down)\n"
                     "    If pointing down will toggle Force/Quote mode and do Force Effect or play Quote accordingly\n"
                     "    If parallel will do Force/Quote\n"
                     "  Start/Stop Tracks = Long Click PWR (pointing up)\n"
                     "    Default track only (use Track Player while OFF to select tracks or playback modes)\n"

                     "Rehearsal / Choreography Modes*\n"
                     "  Begin Rehearsal = While Off, Hold AUX + Twist\n"
                     "      If a Saved Rehearsal Exists it will prompt you to \"Replace?\"\n"
                     "      To confirm Turn the hilt Right (Clockwise) to \"Accept\" and Click PWR to begin a new Rehearsal\n"
                     "      To keep saved rehearsal Click AUX and Rehearsal Mode will be cancelled.\n"
                     "    Saber will Ignite in Rehearsal Mode\n"
                     "    In Rehearsal Mode, standard Clash and Lockup controls are used to record sequence\n"
                     "  Clash = Clash\n"
                     "  Hold PWR + Clash = Lockup\n"
                     "    Rehearsal will also record the sound files used for each effect to repeat in Choreography\n"
                     "  Cancel Rehearsal Mode = Hold AUX\n"
                     "  Save Rehearsal = Hold PWR\n"
                     "  Begin Choreography = While Off, Hold AUX - or - Hold AUX + Swing\n"
                     "    During Choreography Mode Clashes, Lockups and sound files are replayed in sequence\n"
                     "    When recorded sequence completes the saber goes into Battle Mode automatically\n"
                     "    If no saved rehearsal is available for font saber will ignite in Battle Mode*\n"
                     "    During Choreography PWR button is disabled\n"
                     "  Turn Off = Hold AUX + Hold PWR\n"
                     "\n"
                     "Edit Mode\n"
                     "  Enter Edit Mode = While Off, Hold AUX + Hold PWR\n"
                     "    If menu prompt wav files are missing from preset you will get \"Error in Font Directory\" warning, refer to Edit Mode setup and requirements\n"
                     "  While in Edit Mode controls are as follows:\n"
                     "    Rotate Forward, Increase Value, Confirm \"Yes\" = Turn Right (Stepped)\n"
                     "      Increment by 5 (Fonts, Tracks, Blade Length) = Hold PWR + Turn Right\n"
                     "      Increment by 500 (Ignition Time, Ignition Delay, Retraction Time, Retraction Delay) = Hold PWR + Turn Right\n"
                     "      Increment by 5000 (Ignition Option2, Retraction Option2) = Hold PWR + Turn Right\n"
                     "    Rotate Back, Decrease Value, Confirm \"No\" = Turn Left (Stepped)\n"
                     "      Increment by 5 (Fonts, Tracks, Blade Length) = Hold PWR + Turn Left\n"
                     "      Increment by 500 (Ignition Time, Ignition Delay, Retraction Time, Retraction Delay) = Hold PWR + Turn Left\n"
                     "      Increment by 5000 (Ignition Option2, Retraction Option2) = Hold PWR + Turn Left\n"
                     "    Select, Save, Enter = Click PWR\n"
                     "    Cancel, Revert, Go Back = Click AUX\n"
                     "    Go to Main Menu (from sub-menu) - Hold AUX\n"
                     "    Exit Edit Mode - Hold AUX (or rotate to \"Exit\") while in Main Menu\n"
                     "  \"Edit Color\" Additional Control\n"
                     "    \"Color List\" and \"Adjust Color Hue\" Zoom Mode = Hold PWR while turning to Zoom color in, release to save\n"
                     "\n"
                     "Edit Settings\n"
                     "  Enter Edit Settings = While Off, Hold AUX + Hold PWR\n"
                     "    If menu prompt wav files are missing from preset you will get \"Error in Font Directory\" warning, refer to Edit Mode setup and requirements\n"
                     "  While in Edit Mode controls are as follows:\n"
                     "    Rotate Forward, Increase Value, Confirm \"Yes\" = Turn Right (Stepped)\n"
                     "      Increment by 5 (Blade Length) = Hold PWR + Turn Right\n"
                     "    Rotate Back, Decrease Value, Confirm \"No\" = Turn Left (Stepped)\n"
                     "      Increment by 5 (Blade Length) = Hold PWR + Turn Left\n"
                     "    Select, Save, Enter = Click PWR\n"
                     "    Cancel, Revert, Go Back = Click AUX\n"
                     "    Exit Edit Settings - Hold AUX\n"
                     ) : GeneralPage::instance->buttons->num->GetValue() == 1 ? wxString(
                       "Standard Controls While Blade is OFF\n"
                       "  Turn On / Ignite Saber* = Click PWR\n"
                       "  Turn On / Ignite Saber (Muted) = Click + Long Click PWR\n"
                       "  Start / Stop Tracks = Double Click PWR (pointing straight up)\n"
                       "  Track Player = Double Click PWR (parallel or down)\n"
                       "    Turn Right (Stepped) = Next Track\n"
                       "    Turn Left (Stepped) = Previous Track\n"
                       "    Click PWR = Play Current Track Once\n"
                       "    Hold PWR = Random (will play current track and then randomly select next tracks)\n"
                       "    Hold PWR + Turn Right = Rotate (will play current track and then next sequential tracks)\n"
                       "    Hold PWR + Turn Left = Loop Current Track\n"
                       "  Force/Quote Player - Triple Click PWR\n"
                       "    If quotes exist in current font pointing straight down will toggle between Force/Quote\n"
                       "    If parallel will do Force/Quote based on current mode\n"
                       "  Special Abilities (Style Controlled)\n"
                       "    Hold PWR + Turn Right (parallel or up) = Special Ability 5 (USER5)\n"
                       "    Hold PWR + Turn Left (parallel or up) = Special Ability 6 (USER6)\n"
                       "    Hold PWR + Turn Right (pointing down) = Special Ability 7 (USER7)\n"
                       "    Hold PWR + Turn Left (pointing down) = Special Ability 8 (USER8)\n"
                       "  Toggle Spin Mode* = Hold PWR + Swing\n"
                       "    Disables Clash, Stab and Lockup effects to allow for spinning and flourishes\n"
                       "    Will play bmbegin.wav or force.wav when toggled ON/OFF\n"
                       "  Toggle Gesture Sleep = Hold PWR + Clash (pointing down)\n"
                       "    Gestures sleep automatically if Blade Detect is enabled and blade is missing\n"
                       "  Next Preset = Long Click PWR (parallel or up)\n"
                       "  Previous Preset = Long Click PWR (pointing down)\n"
                       "  Scroll Presets (using twist menu) = Hold PWR\n"
                       "    Turn Right (Stepped) = Next Preset\n"
                       "      Increment by 5 = Hold PWR + Turn Right\n"
                       "    Turn Left (Stepped) = Previous Preset\n"
                       "      Increment by 5 = Hold PWR + Turn Left\n"
                       "    Click PWR = Select Preset\n"
                       "    Hold PWR = Select and Ignite Preset\n"
                       "    Long Click PWR = First Preset\n"
                       "  Volume Menu = Hold PWR + Clash (parallel or up)\n"
                       "    Turn Right (Stepped) = Increase Volume (to max)\n"
                       "    Turn Left (Stepped) = Decrease Volume (to min)\n"
                       "    Click PWR = Exit\n"
                       "  Battery Level = Double Click + Long Click PWR\n"
                       "  Change Font\n"
                       "    Next Font = Triple Click + Long Click PWR (parallel or up)\n"
                       "    Previous Font = Triple Click + Long Click PWR (down)\n"
                       "  Copy Preset = Quadruple (Four) Click + Hold PWR\n"
                       "\n"
                       "Standard Controls While Blade is ON\n"
                       "  Turn Off / Retract Blade = Hold PWR\n"
                       "  Clash Effect = Clash Saber\n"
                       "  Lockup Effect = Hold PWR + Clash\n"
                       "  Stab Effect = Stab (thrust with impact at tip of blade)\n"
                       "  NEW Control! Drag Effect = Hold PWR + Stab (pointing straight down)\n"
                       "  Melt Effect = Hold PWR + Stab (parallel or up)\n"
                       "  Lightning Block = Double Click and Hold PWR\n"
                       "  Blast Effect = Click / Double Click / Triple Click PWR\n"
                       "  Multi-Blast Mode = Hold PWR + Swing\n"
                       "    Each Swing in Multi-Blast Mode will deflect Blast effect\n"
                       "    To exit, click PWR or do Clash\n"
                       "  Force/Quote = Long Click PWR\n"
                       "    If pointing down will toggle Force/Quote mode and do Force Effect or play Quote accordingly\n"
                       "    If parallel will do Force/Quote\n"
                       "  Stop Track - Double Click PWR\n"
                       "    To start/select track saber must be OFF\n"
                       "  Color Change = 4 Clicks PWR (parallel or down)\n"
                       "    Click PWR to save\n"
                       "    Color Zoom* = Double Click and Hold PWR, Release to Save\n"
                       "      For Color List or ColorWheel you can Hold PWR down to zoom in color for easier selection\n"
                       "      Release PWR to save\n"
                       "  Power Save = 4 Clicks PWR (pointing straight up)\n"
                       "  Multi-Phase Preset Change*\n"
                       "    Hold PWR + Twist (parallel or up) =  Next Preset \n"
                       "    Hold PWR + Twist (pointing down) = Previous Preset\n"
                       "  Special Abilities (Style Controlled) (requires FETT263_SPECIAL_ABILITIES)\n"
                       "    Hold PWR + Turn Right (parallel or up) = Special Ability 1 (USER1)\n"
                       "    Hold PWR + Turn Left (parallel or up) = Special Ability 2 (USER2)\n"
                       "    Hold PWR + Turn Right (pointing down) = Special Ability 3 (USER3)\n"
                       "    Hold PWR + Turn Left (pointing down) = Special Ability 4 (USER4)\n"
                       "  Change Style (All Blades)\n"
                       "    Next Style = Triple Click + Long Click PWR (parallel or up)\n"
                       "    Previous Style = Triple Click + Long Click PWR (down)\n"
                       "\n"
                       "\"Battle Mode\" Controls - While ON\n"
                       "  Enter/Exit Battle Mode = Triple Click and Hold PWR\n"
                       "  Clash / Lockup = controlled by gesture\n"
                       "    Clash blade\n"
                       "      If blade swings through the clash it will do a \"glancing Clash\"\n"
                       "      If using FETT263_BM_CLASH_DETECT 6 define (Battle Mode 2.0) normal clash used for hits below the\n"
                       "        FETT263_BM_CLASH_DETECT value (1 ~ 8), if undefined or equal to 0 then Battle Mode 1.0 is used.\n"
                       "      If blade stops/slows on clash the saber will initiate Begin Lockup\n"
                       "      To perform a \"clash\" do an immediate Pull Away this will transition from Begin Lockup to End Lockup in quick succession\n"
                       "      To Lockup, steady the blade after Clash\n"
                       "      To end Lockup do Pull Away\n"
                       "  Drag / Melt = controlled by gesture\n"
                       "    Stab (thrust with impact at tip of blade)\n"
                       "      If pointing down Drag will initiate\n"
                       "      To end Drag pull blade up from floor at an angle\n"
                       "      If parallel or up Melt will initiate\n"
                       "      To end Melt pull blade away from object at an angle\n"
                       "  Blast Effect = Click PWR\n"
                       "    After Blast, swing within 2 seconds to enter Multi-Blast Mode\n"
                       "  Lightning Block = Double Click and Hold PWR\n"
                       "  Force Push = Push Saber\n"
                       "\n"
                       "Rehearsal / Choreography Modes*\n"
                       "  Begin Rehearsal = While Off, Triple Click and Hold PWR\n"
                       "        If a Saved Rehearsal Exists it will prompt you to \"Replace?\"\n"
                       "        To confirm Turn the hilt Right (Clockwise) to \"Accept\" and Click PWR to begin a new Rehearsal\n"
                       "        To keep saved rehearsal Click AUX and Rehearsal Mode will be cancelled.\n"
                       "    Saber will Ignite in Rehearsal Mode\n"
                       "    In Rehearsal Mode, standard Clash and Lockup controls are used to record sequence\n"
                       "  Clash = Clash\n"
                       "  Hold PWR + Clash = Lockup\n"
                       "    Rehearsal will also record the sound files used for each effect to repeat in Choreography\n"
                       "  Cancel Rehearsal Mode = Triple Click and Hold PWR\n"
                       "  Save Rehearsal = Hold PWR\n"
                       "  Begin Choreography = While Off, Hold PWR + Swing\n"
                       "    During Choreography Mode Clashes, Lockups and sound files are replayed in sequence\n"
                       "    When recorded sequence completes the saber goes into Battle Mode automatically\n"
                       "    If no saved rehearsal is available for font saber will ignite in Battle Mode*\n"
                       "    *may vary by define\n"
                       "  End Choreography = Hold PWR\n"
                       "\n"
                       "Edit Mode\n"
                       "    Requires /common folder with all menu prompt sounds\n"
                       "  Enter Edit Mode = While Off, Double Click and Hold PWR\n"
                       "    If menu prompt wav files are missing from preset you will get \"Error in Font Directory\" warning, refer to Edit Mode setup and requirements\n"
                       "\n"
                       "  While in Edit Mode controls are as follows:\n"
                       "    Rotate Forward, Increase Value, Confirm \"Yes\" = Turn Right (Stepped)\n"
                       "      Increment by 5 (Fonts, Tracks, Blade Length) = Hold PWR + Turn Right\n"
                       "      Increment by 500 (Ignition Time, Ignition Delay, Retraction Time, Retraction Delay) = Hold PWR + Turn Right\n"
                       "      Increment by 5000 (Ignition Option2, Retraction Option2) = Hold PWR + Turn Right\n"
                       "    Rotate Back, Decrease Value, Confirm \"No\" = Turn Left (Stepped)\n"
                       "      Increment by 5 (Fonts, Tracks, Blade Length) = Hold PWR + Turn Left\n"
                       "      Increment by 500 (Ignition Time, Ignition Delay, Retraction Time, Retraction Delay) = Hold PWR + Turn Left\n"
                       "      Increment by 5000 (Ignition Option2, Retraction Option2) = Hold PWR + Turn Left   \n"
                       "    Select, Save, Enter = Click PWR\n"
                       "    Cancel, Revert, Go Back = Long Click PWR\n"
                       "    Go to Main Menu (from sub-menu) - Hold PWR\n"
                       "    Exit Edit Mode - Hold PWR (or rotate to \"Exit\") while in Main Menu\n"
                       "\n"
                       "  \"Edit Color\" Additional Control\n"
                       "    \"Color List\" and \"Adjust Color Hue\" Zoom Mode = Double Click and Hold PWR while turning to Zoom color in, release to save\n"
                       "\n"
                       "Edit Settings (Settings Only version of Edit Mode)\n"
                       "    Requires /common folder with all menu prompt sounds\n"
                       "  Enter Edit Mode = While Off, Double Click and Hold PWR\n"
                       "    If menu prompt wav files are missing from preset you will get \"Error in Font Directory\" warning, refer to Edit Mode setup and requirements\n"
                       "\n"
                       "  While in Edit Settings controls are as follows:\n"
                       "    Rotate Forward, Increase Value, Confirm \"Yes\" = Turn Right (Stepped)\n"
                       "      Increment by 5 (Blade Length) = Hold PWR + Turn Right\n"
                       "    Rotate Back, Decrease Value, Confirm \"No\" = Turns Left (Stepped)\n"
                       "      Increment by 5 (Blade Length) = Hold PWR + Turn Left\n"
                       "    Select, Save, Enter = Click PWR\n"
                       "    Cancel, Revert, Go Back = Long Click PWR\n"
                       "    Exit Edit Settings - Hold PWR\n"
                       ) : wxString("Button Configuration Not Supported\n")) +
                "\n"
                "Voice Prompts and sounds required for certain features and should be included\n"
                "in /common folder or /font folder on SD card.\n"
                "\n"
                "Track Player requires track files to be located in /font/tracks for font specific tracks\n"
                "or /common/tracks for universal (all presets) or a combination of the two.";
            break;
          case Configuration::SaberProp::SHTOK:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() == 0 ? wxString(
                     "Activate Muted - None\n"
                     "Activate blade - forward or backward horizontal thrust movement or sharp swing movement (Swing On)\n"
                     "Play/Stop Music - hit the saber (perform clash event) holding the blade up while it's OFF\n"
                     "Turn the blade off - twist the saber like a bike handle holding the saber horizontally or blade down\n"
                     "Next Preset - slightly shake the saber like a soda can while blade is OFF (hold the saber vertically blade up in range up to 30 degrees tilt)\n"
                     "Previous Preset - None\n"
                     "Lockup - automatic by default (Battle Mode) - activates when clash happens and keeps active until swing is registered\n"
                     "Drag - None\n"
                     "Blaster Blocks - None\n"
                     "Force Effects - perform a \"push\" gesture holding the saber vertically or horizontally perpendicular to the arm\n"
                     "Enter Color Change mode - slightly shake the saber like a soda can while blade is ON (hold the saber vertically blade up in range up to 30 degrees tilt)\n"
                     "Confirm selected color and exit Color Change mode - while in Color Change mode hit the saber (perform clash event)\n"
                     "Melt - None\n"
                     "Lightning Block - None\n"
                     "Enter Multi-Block mode - None\n"
                     ) : GeneralPage::instance->buttons->num->GetValue() == 1 ? wxString(
                       "Activate Muted - fast double click while OFF\n"
                       "Activate blade - short click while OFF or forward thrust movement + hit or forward thrust movement or sharp swing movement (Swing On)\n"
                       "Play/Stop Music - hold 1 second and release while ON\n"
                       "Turn the blade off - hold and wait till blade is off while ON (like in Plecter boards) or twist the saber like a bike handle\n"
                       "Next Preset - hold 1 second and release while OFF\n"
                       "Previous Preset - hold and wait while OFF\n"
                       "Lockup - hold + hit clash while ON\n"
                       "Drag - hold + hit clash while ON pointing the blade tip down\n"
                       "Blaster Blocks - short click while ON\n"
                       "Force Effects - hold the button + perform \"push\" gesture holding the hilt vertically\n"
                       "Enter Color Change mode - hold the button + twist the hilt then release the button while ON\n"
                       "Confirm selected color and exit Color Change mode - hold the button until confirmation sound\n"
                       "Melt - hold the button + stab while ON\n"
                       "Lightning Block - fast double click + hold the button while ON\n"
                       "Enter Multi-Block mode - hold the button + swing the saber and release the button while ON (now swing the saber, blaster blocks will trigger automatically)\n"
                       "Exit Multi-Block mode - short click the button while ON\n"
                       "Enter Battle Mode - hold the button + use \"Gesture/Swing Ignition\" then release the button while OFF (blade will turn ON in Battle Mode)\n"
                       "Exit Battle Mode - turn the blade OFF\n"
                       "Enter Volume Menu - hold + clash while OFF\n"
                       "Volume DOWN - hold and release while in Volume Menu\n"
                       "Volume UP - short click while in Volume Menu\n"
                       "Exit Volume Menu - hold + clash while OFF and in Volume Menu\n"
                       "Battery Level - triple click while OFF\n"
                       ) : GeneralPage::instance->buttons->num->GetValue() == 2 || GeneralPage::instance->buttons->num->GetValue() == 3 ? wxString (
                       "Activate Muted - fast double click Activation button while OFF\n"
                       "Activate blade - short click Activation button while OFF or forward thrust movement + hit or forward thrust movement or sharp swing movement (Swing On)\n"
                       "Play/Stop Music - hold 1 second and release Activation button while OFF or ON\n"
                       "Turn the blade off - hold and wait till blade is off while ON (like in Plecter boards) or twist the saber like a bike handle\n"
                       "Next Preset - short click AUX button while OFF\n"
                       "Previous Preset - hold AUX and click Activation button while OFF\n"
                       "Lockup - hold AUX button while ON (like in Plecter boards)\n"
                       "Drag - hold AUX button while ON pointing the blade tip down\n"
                       "Blaster Blocks - short click AUX button while ON\n"
                       "Force Effects - short click Activation button while ON or perform \"push\" gesture holding the hilt vertically\n"
                       "Enter Color Change mode - hold AUX and quickly press and release Activation button while ON then release AUX button\n"
                       "Confirm selected color and exit Color Change mode - hold the button until confirmation sound\n"
                       "Melt - hold AUX (or Activation) button + perform stab action while ON\n"
                       "Lightning Block - hold Activation button + short click AUX button while ON\n"
                       "Enter Multi-Block mode - hold the Activation button + swing the saber and release the button while ON (now swing the saber, blaster blocks will trigger automatically)\n"
                       "Exit Multi-Block mode - short click AUX button while ON\n"
                       "Enter Battle Mode - hold the AUX button + swing the saber and release the button while ON or hold the Activation button + use \"Gesture/Swing Ignition\" then release the button while OFF (blade will turn ON in Battle Mode)\n"
                       "Exit Battle Mode - hold the AUX button + swing the saber and release the button while ON or turn the blade OFF\n"
                       "Enter Volume Menu - long click AUX button while OFF\n"
                       "Volume UP - short click Activation button while OFF and in Volume Menu\n"
                       "Volume DOWN - short click AUX button while OFF and in Volume Menu\n"
                       "Exit Volume Menu - long click AUX button while OFF and in Volume Menu\n"
                       "Battery level - hold AUX button while OFF\n"
                       ) : wxString("Button Configuration Not Supported")) +
                "\n\n"
                "CUSTOM SOUNDS SUPPORTED (add to font to enable):\n"
                "\n"
                "On Demand Power Save - dim.wav\n"
                "On Demand Battery Level - battery.wav\n"
                "Battle Mode On (on toggle) - bmbegin.wav\n"
                "Battle Mode Off (on toggle) - bmend.wav\n"
                "Enter Volume Menu - vmbegin.wav\n"
                "Exit Volume Menu - vmend.wav\n"
                "Force Push - push.wav\n"
                "Fast On (optional) - faston.wav\n"
                "Multi-Blast Mode On - blstbgn.wav\n"
                "Multi-Blast Mode Off - blstend.wav\n";
            break;
          case Configuration::SaberProp::BC:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() == 1 ? wxString(
                     "*************   WHILE SABER BLADE IS OFF   ***************\n"
                     "Turn blade ON         - Short click POW. (or gestures if defined, uses FastOn)\n"
                     "                        * NOTE * Gesture ignitions using FastOn bypass preon.\n"
                     "Turn ON without preon - Short click POW while pointing up.\n"
                     "Turn blade ON Muted   - 4x click and hold POW.\n"
                     "Next Preset           - Long click and release POW.\n"
                     "Prev Preset           - Double click and hold POW, release after a second (click then long click).\n"
                     "Play/Stop Track       - 4x click POW.\n"
                     "Volume Menu:\n"
                     "                      * NOTE * Tilting blade too high or low in Volume Menu will give a warning tone to\n"
                     "                        tilt up or down to avoid erratic rotational volume changes at extreme blade angles.\n"
                     "        Enter/Exit    - Hold POW + Clash.\n"
                     "        Volume UP     - Rotate Right\n"
                     "                      - or -\n"
                     "                      - Long click and release POW while in Volume Menu. (just like next preset)\n"
                     "        Volume DOWN   - Rotate Left\n"
                     "                      - or -\n"
                     "                      - Double click and hold POW, release after a second while in Volume Menu.\n"
                     "                        (click then long click, just like next preset)\n"
                     "        Quick MAX Vol - Short click POW while in Volume Menu.\n"
                     "        Quick MIN Vol - Double click POW while in Volume Menu.\n"
                     "Spoken Battery Level\n"
                     "        in volts      - Triple click POW.\n"
                     "        in percentage - Triple click and hold POW.\n"
                     "On-Demand Batt Level  - Double click POW.\n"
                     "                        and uses battery.wav sound effect.)\n"
                     "\n"
                     "*************   WHILE SABER BLADE IS ON   ****************\n"
                     "Play/Stop Track       - 4x click POW.\n"
                     "Next Preset Fast      - Long click and release POW while pointing up.\n"
                     "Prev Preset Fast      - Double click and hold POW, release after a second while pointing up. (click then long click)\n"
                     "                        * NOTE * Fast switching bypasses preon and font.wav.\n"
                     "Clash                 - No buttons, just hit the blade against something.\n"
                     "                        In Battle Mode, Hold POW and Clash to temporarily\n"
                     "                        override the auto-lockup and do regular Clash.\n"
                     "Stab                  - Just Thrust forward with a stabbing motion.\n"
                     "                        Works in Battle Mode.\n"
                     "Blaster Blocks        - Click or Double click POW.\n"
                     "Spam Blaster Blocks   - 3x click and hold while pointing up. This toggles SPAM BLAST mode ON/OFF,\n"
                     "                        and makes the button super sensitive for multiple blaster blocks.\n"
                     "                        * Note * This gets in the way of normal features,\n"
                     "                        so turn off when you're done spamming.  Plays mzoom.wav.\n"
                     "Auto Swing Blast      - Swinging within 1 second of doing button activated\n"
                     "                        Blaster Block will start this timed mode.\n"
                     "                        To trigger auto blaster blocks, swing saber\n"
                     "                        within 1 second of last Swing Blast block.\n"
                     "                        To exit, stop swinging for 1 second.\n"
                     "Lockup                - Hold POW + Clash.\n"
                     "                        In Battle Mode, just Clash and stay there,\n"
                     "                        pull away or press POW to end lockup.\n"
                     "Drag                  - Hold POW + Clash while pointing down.\n"
                     "Melt                  - No button, just stab something. pull away or\n"
                     "                        press POW to end.\n"
                     "Lightning Block       - Double click and hold POW.\n"
                     "Battle Mode           - Triple click and hold POW to enter and exit.\n"
                     "                        Power OFF is disabled while in Battle Mode,\n"
                     "                        YOU MUST EXIT THE MODE WITH THIS COMBO FIRST.\n"
                     "Force Effect          - Hold POW + Twist. (while NOT pointing up or down)\n"
                     "Monophonic Force      - Hold POW + Twist. (while pointing up)\n"
                     "Color Change Mode     - Hold POW + Twist. (while pointing down)\n"
                     "                        Rotate hilt to cycle through all available colors, or\n"
                     "                        Click POW to change if ColorChange<> used in blade style,\n"
                     "                        Click + hold POW to save color selection and exit.\n"
                     "                        Triple click POW to cancel and restore original color.\n"
                     "Quote Player          - Triple click POW.\n"
                     "Toggle sequential or\n"
                     "  random quote play   - 4x click and hold POW. (while pointing down)\n"
                     "Force Push            - Push hilt perpendicularly from a stop.\n"
                     "Swap (EffectSequence) - 4x click and hold POW. (while NOT pointing up)\n"
                     "                        * Requires EFFECT_USER1 in blade style.\n"
                     "PowerSave Dim Blade   - 4x click and hold POW. (while pointing up)\n"
                     "                        To use Power Save requires AlphaL based EffectSequence in style.\n"
                     "Turn off blade        - Hold POW and wait until blade is off\n"
                     "Turn OFF without postoff - Turn OFF while pointing up.\n"
                     ) : GeneralPage::instance->buttons->num->GetValue() == 2 || GeneralPage::instance->buttons->num->GetValue() == 3 ? wxString(
                       "*************   WHILE SABER BLADE IS OFF   ***************\n"
                       "Turn blade ON         - Short click POW. (or gestures if defined, uses FastOn)\n"
                       "                        * NOTE * Gesture ignitions using FastOn bypass preon.\n"
                       "Turn ON without preon - Short click POW while pointing up.\n"
                       "Turn blade ON Muted   - 4x click and hold POW.\n"
                       "Next Preset           - Long click and release POW.\n"
                       "Prev Preset           - Double click and hold POW, release after a second (click then long click).\n"
                       "Play/Stop Track       - Hold AUX + Double click POW.\n"
                       "Volume Menu:\n"
                       "                      * NOTE * Tilting blade too high or low in Volume Menu will give a warning tone to\n"
                       "                        tilt up or down to avoid erratic rotational volume changes at extreme blade angles.\n"
                       "        Enter/Exit    - Long click AUX.\n"
                       "        Volume UP     - Rotate Right\n"
                       "                      - or -\n"
                       "                      - Long click and release POW while in Volume Menu. (just like next preset)\n"
                       "        Volume DOWN   - Rotate Left\n"
                       "                      - or -\n"
                       "                      - Double click and hold POW, release after a second while in Volume Menu.\n"
                       "                        (click then long click, just like next preset)\n"
                       "        Quick MAX Vol - Short click POW while in Volume Menu.\n"
                       "        Quick MIN Vol - Double click POW while in Volume Menu.\n"
                       "Spoken Battery Level\n"
                       "        in volts      - Triple click POW.\n"
                       "        in percentage - Triple click and hold POW.\n"
                       "On-Demand Batt Level  - Double click POW.\n"
                       "\n"
                       "*************   WHILE SABER BLADE IS ON   ****************\n"
                       "Play/Stop Track       - Hold AUX + Double click POW.\n"
                       "Next Preset Fast      - Hold AUX + Long click and release POW while pointing up.\n"
                       "Prev Preset Fast      - Hold AUX + Double click and hold POW for a second\n"
                       "                        while pointing up. (click then long click)\n"
                       "                        * NOTE * Fast switching bypasses preon and font.wav.\n"
                       "Clash                 - No buttons, just hit the blade against something.\n"
                       "                        In Battle Mode, Hold any button and Clash to\n"
                       "                        temporarily override the auto-lockup and do regular Clash.\n"
                       "Stab                  - Just Thrust forward with a stabbing motion.\n"
                       "                        Works in Battle Mode.\n"
                       "Blaster Blocks        - Click or Double click POW.\n"
                       "Spam Blaster Blocks   - 3x click and hold while pointing up. This toggles SPAM BLAST mode ON/OFF,\n"
                       "                        and makes the button super sensitive for multiple blaster blocks.\n"
                       "                        * Note * This gets in the way of normal features,\n"
                       "                        so turn off when you're done spamming.  Plays mzoom.wav.\n"
                       "Auto Swing Blast      - Swinging within 1 second of doing button activated\n"
                       "                        Blaster Block will start this timed mode.\n"
                       "                        To trigger auto blaster blocks, swing saber\n"
                       "                        within 1 second of last Swing Blast block.\n"
                       "                        To exit, stop swinging for 1 second.\n"
                       "Lockup                - Hold AUX + Clash.\n"
                       "                        In Battle Mode, just Clash and stay there,\n"
                       "                        pull away or press any button to end lockup.\n"
                       "Drag                  - Hold AUX + Clash while pointing down.\n"
                       "Melt                  - No button, just stab something,\n"
                       "                        pull away or press any button to end.\n"
                       "Lightning Block       - Double click and hold POW.\n"
                       "Battle Mode           - Hold POW + Click AUX to enter and exit.\n"
                       "                        Power OFF is disabled while in Battle Mode,\n"
                       "                        YOU MUST EXIT THE MODE WITH THIS COMBO FIRST.\n"
                       "Force Effect          - Hold POW + Twist. (while NOT pointing up or down)\n"
                       "Monophonic Force      - Hold POW + Twist. (while pointing up)\n"
                       "Color Change Mode     - Hold POW + Twist. (while pointing down)\n"
                       "                        Rotate hilt to cycle through all available colors, or\n"
                       "                        Click AUX to change if ColorChange<> used in blade style,\n"
                       "                        Click + hold POW to save color selection and exit.\n"
                       "                        Triple click POW to cancel and restore original color.\n"
                       "Quote Player          - Triple click POW.\n"
                       "Toggle sequential or\n"
                       "  random quote play   - Hold AUX + Twist. (while pointing down)\n"
                       "Force Push            - Push hilt perpendicularly from a stop.\n"
                       "Swap (EffectSequence) - Hold AUX + Twist. (while NOT pointing up)\n"
                       "                        * Requires EFFECT_USER1 in blade style.\n"
                       "PowerSave Dim Blade   - Hold AUX + Twist. (while pointing up)\n"
                       "Turn off blade        - Hold POW and wait until blade is off\n"
                       "Turn OFF without postoff - Turn OFF while pointing up.\n"

                       ) : wxString("Button Configuration Not Supported"));
            break;
          case Configuration::SaberProp::CAIWYN:
            buttons =
                (GeneralPage::instance->buttons->num->GetValue() >= 2 ? wxString(
                     " While saber is OFF:\n"
                     "               Next Preset: Hold Aux for 1 second when track is not playing\n"
                     "             Check Battery: Double-click and hold Aux for 1 second when\n"
                     "                            track is not playing\n"
                     "          Start/Stop Track: Click Aux\n"
                     "                Next Track: Hold Aux for 1 second while track is playing\n"
                     "                            | Tracks must be stored in <font>/tracks/*.wav\n"
                     "                            | and will be selected in alphabetical order.\n"
                     "         Track Player Mode: Double-click and hold Aux for 1 second while\n"
                     "                            track is playing\n"
                     "                            | This cycles through three playback modes:\n"
                     "                            | 1. Play a single track and stop (default)\n"
                     "                            | 2. Repeat a single track in a loop\n"
                     "                            | 3. Repeat all tracks in a loop\n"
                     "             Turn Saber On: Press Power\n"
                     "     Turn On & Start Track: Hold Aux or Aux2 and press Power\n"
                     "\n"
                     " While saber is ON:\n"
                     "             Blaster Block: Click Aux\n"
                     "                    Lockup: Hold Aux or Aux2 during impact\n"
                     "                      Drag: Hold Aux or Aux2 during impact with blade pointed\n"
                     "                            down.\n"
                     "                      Melt: Hold Aux or Aux2 and stab\n"
                     "           Lightning Block: Click/Hold Power\n"
                     "\n"
                     "         Enter Volume Menu: Double-click and hold Aux for 1 second\n"
                     "                            | Be aware that the first click will trigger a\n"
                     "                            | blaster block.\n"
                     "           Increase Volume: Rotate hilt right while in Volume Menu\n"
                     "           Decrease Volume: Rotate hilt left while in Volume Menu\n"
                     "   Save & Exit Volume Menu: Click Power\n"
                     "  Reset & Exit Volume Menu: Click Aux\n"
                     "\n"
                     "   Enter Color Change Mode: Triple-click and hold Aux for 1 second\n"
                     "                            | Be aware that the first two clicks will\n"
                     "                            | trigger blaster blocks.\n"
                     "              Change Color: Rotate hilt while in Color Change Mode\n"
                     "                Color Zoom: Press & Hold Power and Rotate hilt\n"
                     "                            | This will fine-tune the selected color before\n"
                     "                            | saving your change.\n"
                     "  Save & Exit Color Change: Release Power while in Color Zoom\n"
                     " Reset & Exit Color Change: Click Aux\n"
                     "\n"
                     "            Turn Saber Off: Hold Aux or Aux2 and press Power\n"
                           ) : wxString("Button Configuration Not Supported"));
                break;
        }

        auto buttonDialog = wxDialog(
            MainWindow::instance,
            wxID_ANY,
            "Prop File Buttons",
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP
            );
        auto textSizer = new wxBoxSizer(wxVERTICAL);
        textSizer->Add(buttonDialog.CreateTextSizer(buttons), wxSizerFlags(0).Border(wxALL, 10));
        buttonDialog.SetSizer(textSizer);
        buttonDialog.DoLayoutAdaptation();
        buttonDialog.ShowModal();
      }, ID_Buttons);
}

void PropPage::updateProps() {
  auto lastSelect = propSelection->GetStringSelection();
  propSelection->Clear();
  propSelection->Append("Default");
  for (const auto& prop : AppState::instance->getProps()) {
    propSelection->Append(prop.getName());
  }
  if ([&]() { for (const auto& prop : propSelection->GetStrings()) if (prop == lastSelect) return true; return false; }()) {
    propSelection->SetStringSelection(lastSelect);
  } else propSelection->SetStringSelection("Default");
}

void PropPage::updatePropSelection() {
  for (auto& prop : AppState::instance->getProps()) {
    prop.show(propSelection->GetStringSelection() == prop.getName());
  }
}

void PropPage::update() {
  for (auto& prop : AppState::instance->getProps()) {
    if (propSelection->GetStringSelection() != prop.getName()) continue;

    for (auto& [ name, setting ] : prop.getSettings()) {
      for (const auto& disable : setting.disables) {
        auto key = prop.getSettings().find(disable);
        if (key == prop.getSettings().end()) continue;

        key->second.disabled = !setting.getOutput().empty();
      }
    }

    for (auto& [ name, setting ] : prop.getSettings()) {
#     define CHECKENABLED !setting.disabled && setting.checkRequiredSatisfied(prop.getSettings())
      switch(setting.type) {
        case PropFile::Setting::SettingType::TOGGLE:
          setting.toggle->Enable(CHECKENABLED);
          break;
        case PropFile::Setting::SettingType::NUMERIC:
          setting.numeric->Enable(CHECKENABLED);
          break;
        case PropFile::Setting::SettingType::DECIMAL:
          setting.decimal->Enable(CHECKENABLED);
          break;
        case PropFile::Setting::SettingType::OPTION:
          setting.option->Enable(CHECKENABLED);
          break;
      }
#     undef CHECKENABLED
    }
  }
}
