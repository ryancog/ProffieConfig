// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "mainmenu/dialogs/addconfig.h"
#include "onboard/onboard.h"

#include "mainmenu/mainmenu.h"
#include "editor/editorwindow.h"
#include "editor/pages/propspage.h"
#include "editor/pages/bladespage.h"
#include "editor/pages/presetspage.h"
#include "editor/dialogs/bladearraydlg.h"

#include <wx/menu.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif
#include <wx/sizer.h>
#include <wx/statbox.h>

std::vector<bool*> Onboard::Overview::eventRunTrackers{};
#define EVENT_PAGE_SETUP \
  event.Skip(); \
  static bool hasRun{false}; \
  if (std::find(eventRunTrackers.begin(), eventRunTrackers.end(), &hasRun) == eventRunTrackers.end()) eventRunTrackers.push_back(&hasRun); \
  if (hasRun) return; \
  hasRun = true;

Onboard::Overview::Overview(wxWindow* parent) : wxWindow(parent, ID_Overview) {
  sizer = new wxBoxSizer(wxVERTICAL);

  generateNewPage("Introduction to ProffieConfig",

                  "ProffieConfig's main menu has just opened up to the right. This page will serve as\n"
                  "the instructions for the introduction, but all the instructions will be referencing\n"
                  "actions to be completed on the main menu and any windows you open along the way.\n"
                  "\n"
                  "Here you can add and manage configurations, apply a configuration to a Proffieboard,\n"
                  "and open the Serial Monitor to connect to the Proffieboard if needed.\n"
                  "\n"
                  "Located up top under \"File\" (currently disabled) you can re-run this setup later at any time.\n"
                  "Under \"Help\" you can also find a link to report any issues you have with the app to me.\n"
                  "\n\n"
                  "Go ahead and click on \"Add\" to add your first configuration.\n"
                  );
  mainMenuDisables[MainMenu::ID_AddConfig] = false;

  SetSizerAndFit(sizer);
}
Onboard::Overview::~Overview() {
  if (guideMenu != nullptr) guideMenu->Close(true);
}

void Onboard::Overview::prepare() {
  guideMenu = new MainMenu(this);

  auto updateGuideMenuLocation{[=](){
          auto parentRect = GetParent()->GetScreenRect();
          guideMenu->SetPosition(wxPoint(parentRect.x + parentRect.width + 20, parentRect.y + ((parentRect.height - guideMenu->GetSize().y) / 2)));
                           }};
  Bind(wxEVT_SIZE, [=](wxSizeEvent&) { updateGuideMenuLocation(); });
  GetParent()->Bind(wxEVT_MOVE, [=](wxMoveEvent&) { updateGuideMenuLocation(); });
  guideMenu->Bind(wxEVT_MOVE, [=](wxMoveEvent&) { updateGuideMenuLocation(); });

  for (auto hasRun : eventRunTrackers) *hasRun = false;
  prepareMainMenu();
  linkMainMenuEvents();
}
void Onboard::Overview::prepareMainMenu() {
  guideMenu->GetMenuBar()->Disable();
  guideMenu->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    if (event.CanVeto()) {
      event.Veto();
      wxMessageDialog(guideMenu, "You cannot close this during First-Time Setup.", "Close ProffieConfig", wxOK | wxCENTER).ShowModal();
    }
  });
  guideMenu->Bind(wxEVT_UPDATE_UI, [&](wxUpdateUIEvent& event) {
    event.Skip();
    for (const auto& [ id, disabled ] : mainMenuDisables) {
      guideMenu->FindWindowById(id)->Enable(!disabled);
    }
  });
}
void Onboard::Overview::prepareEditor() {
  guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_ExportConfig, false);
  guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_StyleEditor, false);
  guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_VerifyConfig, false);
  guideMenu->activeEditor->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    if (doneWithEditor) {
      event.Skip();
      return;
    }
    if (event.CanVeto()) {
      event.Veto();
      wxMessageDialog(guideMenu->activeEditor, "You cannot close this during First-Time Setup.", "Close ProffieConfig Editor", wxOK | wxCENTER).ShowModal();
    }
  });
}

void Onboard::Overview::linkMainMenuEvents() {
  guideMenu->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      event.Skip(false);

      generateNewPage("Adding a Configuration",

                      "Now, from this new window, you can either create a new config, or import one you already have!\n"
                      "For now, just create a new one, you can import your own later.\n"
                      "\n"
                      "Your configurations must have unique names, but later you can import as many as you'd like.\n"
                      "\n"
                      "If you bought your saber from someone else, they should have provided you a configuration.\n"
                      "\n"
                      "These files end with \".h\" (this may not show up depending on your computer settings) if you're\n"
                      "looking for them on your computer to import, and contain all the information needed to make your\n"
                      "Proffieboard work!\n"
                      );
      auto configDialog = AddConfig(guideMenu);
      static_cast<wxToggleButton*>(configDialog.FindWindow(AddConfig::ID_ImportExisting))->Disable();
      configDialog.ShowModal();

    }, MainMenu::ID_AddConfig);
  guideMenu->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      generateNewPage("Edit Configuration",

                      "If you import a config later, ProffieConfig will manage it, and you won't need\n"
                      "the file after you do so, though it's never a bad idea to keep backups.\n"
                      "\n"
                      "We'll go over how to export your configuration later, which could also be useful if\n"
                      "you ever need help troubleshooting.\n"
                      "\n\n"
                      "Click on \"Edit Selected Configuration\" in order to open your new config\n"
                      "in the ProffieConfig editor.\n"
                      "(Notice it's been selected in the drop-down)\n"
                      );
      mainMenuDisables.at(MainMenu::ID_AddConfig) = true;
      mainMenuDisables.at(MainMenu::ID_EditConfig) = false;

    }, MainMenu::ID_ConfigSelect);
  guideMenu->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP
        generateNewPage("Configuration - General Settings",

                        "This is the ProffieConfig editor, where you can tweak virtually every aspect of how\n"
                        "your Proffieboard operates, and set up everything to your liking.\n"
                        "\n"
                        "This first page is the \"General\" page, where there's some more basic\n"
                        "options for you to configure.\n"
                        "\n"
                        "If you're unsure what an option does, hover over it with your mouse, and\n"
                        "a little tooltip will appear to explain the setting in more detail.\n"
                        "(This goes for almost every setting in ProffieConfig)\n"
                        "\n\n"
                        "When you're done exploring, switch the page to \"Prop File\" with\n"
                        "the drop down at the top.\n"
                        );
      mainMenuDisables[MainMenu::ID_EditConfig] = true;

      guideMenu->activeEditor->windowSelect->entry()->Clear();
      guideMenu->activeEditor->windowSelect->entry()->Append("General");
      guideMenu->activeEditor->windowSelect->entry()->Append("Prop File");
      guideMenu->activeEditor->windowSelect->entry()->SetSelection(0);

      linkEditorEvents();
      prepareEditor();

    }, MainMenu::ID_EditConfig);
}

void Onboard::Overview::linkEditorEvents() {
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_UPDATE_UI, [&](wxUpdateUIEvent& event) {
    event.Skip();
    for (const auto& [ id, disabled ] : bladeDisables) {
      guideMenu->activeEditor->bladesPage->GetStaticBox()->FindWindow(id)->Enable(!disabled);
    }
  });

  guideMenu->activeEditor->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      if (guideMenu->activeEditor->windowSelect->entry()->GetSelection() != 1) {
        hasRun = false;
        return;
      }

      generateNewPage("Configuration - Prop File",

                      "In order to change the controls for your Proffieboard, you can choose\n"
                      "what is known as a prop file.\n"
                      "\n"
                      "In ProffieOS, prop files are what control the way you interact with your\n"
                      "saber. Each one is different, and has it's own set of settings you can\n"
                      "configure.\n"
                      "\n"
                      "These prop files are made by various members of the community, and they are\n"
                      "typically listed by their creator name/username.\n"
                      "\n\n"
                      "While the default is a solid place to start, go ahead and select a\n"
                      "different one from the drop-down.\n"
                      );
      guideMenu->activeEditor->propsPage->buttonInfo->Disable();
      guideMenu->activeEditor->propsPage->propInfo->Disable();

    }, EditorWindow::ID_WindowSelect);
  guideMenu->activeEditor->propsPage->GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Prop File",

                      "Now that you've chosen another prop file, you should see some new settings.\n"
                      "\n"
                      "If not, this prop file may not have any (may just be custom buttons/controls).\n"
                      "You can also choose a different prop file if you'd like to explore.\n"
                      "\n\n"
                      "Once you've configured some settings (if any), go ahead and press \"Buttons...\"\n"
                      );
      guideMenu->activeEditor->propsPage->buttonInfo->Enable();
      guideMenu->activeEditor->propsPage->propInfo->Enable();

    }, PropsPage::ID_PropSelect);
  guideMenu->activeEditor->propsPage->GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Prop File",

                      "This shows all the controls for your saber with the specified configuration and\n"
                      "prop file setup.\n"
                      "\n"
                      "Some prop files might not support all types of button configurations, and that\n"
                      "will be noted here too.\n"
                      "\n\n"
                      "You can close out of this window, and once you've gotten a chance to\n"
                      "test out some props/settings, switch the page to \"Blade Arrays\"\n"
                      "to continue configuring.\n"
                      );
      guideMenu->activeEditor->windowSelect->entry()->Append("Blade Arrays");

    }, PropsPage::ID_Buttons);
  guideMenu->activeEditor->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      if (guideMenu->activeEditor->windowSelect->entry()->GetSelection() != 2) {
        hasRun = false;
        return;
      }

      generateNewPage("Configuration - Blade Arrays",

                      "This page is where we'll set up all the \"blades\" on your saber.\n"
                      "\n"
                      "Somewhat confusingly, pretty much anything that lights up on your saber\n"
                      "is known as a \"blade\" in ProffieOS.\n"
                      "\n"
                      "This includes any accent LEDs, illuminated connectors, illuminated\n"
                      "switches, etc.\n"
                      "\n"
                      "This guide will show you the different blade types and their features, but\n"
                      "your saber doesn't need to (and most likely won't) use all of them.\n"
                      "\n\n"
                      "Select \"Blade 0\" to configure it.\n"
                      "(Blades begin counting from 0, not 1)\n"
                      );

      guideMenu->activeEditor->bladesPage->bladeArray->Disable();
      guideMenu->activeEditor->bladesPage->addBladeButton->Disable();
      guideMenu->activeEditor->bladesPage->bladeArrayButton->Disable();

    }, EditorWindow::ID_WindowSelect);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Blade Arrays",

                      "The default type for a blade is \"WS281X\", commonly referred to as a\n"
                      "\"Pixel\" blade, which has many individually-controllable LEDs.\n"
                      "\n"
                      "For these blades, you can choose the Color Order, the Data Pin, and the number\n"
                      "of pixels in the blade.\n"
                      "\n"
                      "Chances are you won't need to change the Color Order, almost every single WS281X blade\n"
                      "uses \"GRB\". If you're unsure, simply leave the color order as it is.\n"
                      "\n"
                      "The data pin is the physical pin your blade is connected to on the Proffieboard for\n"
                      "sending the, well, data. You can select an option from the drop-down, or type directly\n"
                      "in the box to specify any other (supported) pin on the Proffieboard.\n"
                      "\n"
                      "Once you're familiar with these settings, click the \"+\" icon under the \"SubBlades\"\n"
                      "list and select the created SubBlade (\"SubBlade 0\").\n"
                      );
      bladeDisables.at(BladesPage::ID_AddSubBlade) = false;

    }, BladesPage::ID_BladeSelect);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Blade Arrays",

                      "SubBlades are specific to WS281X blades, allowing you to seperate one blade into multiple\n"
                      "effective blades, and you'll notice mostly the same controls, but with a few extra specific\n"
                      "to WS281X blades in SubBlade mode.\n"
                      "\n"
                      "There's a few different types of SubBlade setups to choose from which you can read about by\n"
                      "hovering over the option. (\"Standard\", \"Stride\", and \"ZigZag\")\n"
                      "\n"
                      "With the standard type, the range for numbers starts at 0 and ends at one less than\n"
                      "\"Number of Pixels\".\n"
                      "The end of one SubBlade should also not overlap with the start of another,\n"
                      "though these SubBlades don't have to be in order. (e.g. the physical end of a WS281X\n"
                      "blade could be the first SubBlade)\n"
                      "\n"
                      "When SubBlade mode is active, only SubBlade 0 will show blade controls, the\n"
                      "rest will just be the settings for the specific SubBlade (if there are any).\n"
                      "\n"
                      "Now, try changing the type of Blade to a \"Tri-LED Star\"\n"
                      );
      bladeDisables.at(BladesPage::ID_AddSubBlade) = true;
      bladeDisables.at(BladesPage::ID_BladeType) = false;
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Clear();
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Append("WS281X (RGB)");
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Append("WS281X (RGBW)");
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Append("Tri-LED Star");
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Append("Quad-LED Star");
      guideMenu->activeEditor->bladesPage->bladeType->entry()->SetSelection(0);

    }, BladesPage::ID_SubBladeSelect);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      if (guideMenu->activeEditor->bladesPage->bladeType->entry()->GetSelection() != 2) {
        hasRun = false;
        return;
      }

      generateNewPage("Configuration - Blade Arrays",

                      "Another common type of blade is a Tri-LED (or even Quad-LED, for which there's another\n"
                      "option in the drop-down) Star, commonly referred to as an \"In-Hilt\" configuration.\n"
                      "\n"
                      "For this setup, you specify the color of the each LED and the size of resistor\n"
                      "(in Ohms) you placed on the power line going to each LED.\n"
                      "\n"
                      "The LEDs correspond sequentially to the selected Power Pins, and you should select\n"
                      "a number of Power Pins equal to the number of LEDs you're setting up.\n"
                      "(e.g. if Power Pins 2, 3, and 4 are selected, Pin 2 would go to LED1,\n"
                      "Pin 3 would go to LED 2 and Pin 4 would go to LED 3)\n"
                      "\n"
                      "You can also add custom power pins by typing them into the text box, then clicking the \"+\".\n"
                      "\n"
                      "Finally, check out the \"Single Color\" blade type.\n"
                      );
      guideMenu->activeEditor->bladesPage->bladeType->entry()->Append("Single Color");

    }, BladesPage::ID_BladeType);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      if (guideMenu->activeEditor->bladesPage->bladeType->entry()->GetSelection() != 4) {
        hasRun = false;
        return;
      }

      generateNewPage("Configuration - Blade Arrays",

                      "The final blade type is a Single Color blade. This is useful is you have\n"
                      "a \"dumb\" LED accent on your saber, a single-color illuminated switch, or similar\n"
                      "\n"
                      "When creating bladestyles for a Single Color blade, the color should be set to\n"
                      "WHITE (or equivalent dimmed value), we'll go over bladestyles in a bit.\n"
                      "\n"
                      "Now that we've covered the basics of blades, you should know you can add as many\n"
                      "blades as you'd like, provided you have the pins on the Proffieboard, of course!\n"
                      "\n"
                      "For WS281X blades, Power Pins can be shared, which can be useful if you're running low.\n"
                      "\n"
                      "Click \"Blade Awareness...\" to continue.\n"
                      );
      guideMenu->activeEditor->bladesPage->bladeArrayButton->Enable();

    }, BladesPage::ID_BladeType);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Blade Awareness",

                      "Blade Awareness is a neat little feature of ProffieOS that, if your saber is installed\n"
                      "in such a way that supports it, allows the saber to respond to what blade is inserted,\n"
                      "or none at all, provided the blade also supports the feature.\n"
                      "\n"
                      "There two main \"elements\" to Blade Awareness, if you will: \"Blade Detect\" and \"Blade ID\",\n"
                      "but ultimately each will, once enabled, create new Blade Arrays which can each have their own\n"
                      "blade configuration and set of presets.\n"
                      "\n"
                      "There's a lot which can be configured here, (currently disabled) and if you're interested\n"
                      "in setting it up, it's recommended to read through what each setting does and how it\n"
                      "works by reading the tool tip when hovering over each setting.\n"
                      "\n"
                      "Close out of this dialog and switch the page to \"Presets And Styles\" to continue."
                      );
      guideMenu->activeEditor->bladesPage->bladeArrayDlg->enableDetect->Disable();
      guideMenu->activeEditor->bladesPage->bladeArrayDlg->enableID->Disable();

    }, BladesPage::ID_OpenBladeArrays);
  guideMenu->activeEditor->bladesPage->bladeArrayDlg->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    EVENT_PAGE_SETUP;
    guideMenu->activeEditor->windowSelect->entry()->Append("Presets And Styles");
  });
  guideMenu->activeEditor->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;
      if (guideMenu->activeEditor->windowSelect->entry()->GetSelection() != 3) {
        hasRun = false;
        return;
      }

      guideMenu->activeEditor->bladesPage->bladeType->entry()->SetSelection(0);
      generateNewPage("Configuration - Presets and Styles",

                      "Presets are arguably the most important part of the configuration.\n"
                      "\n"
                      "Presets contain all the information about what to actually do with all your LEDs\n"
                      "and what kind of effects should happen on your saber.\n"
                      "\n"
                      "First things first, create a new preset by clicking the \"+\".\n"""
                      );
      guideMenu->activeEditor->presetsPage->bladeArray->Disable();
      guideMenu->activeEditor->presetsPage->bladeList->Disable();
      guideMenu->activeEditor->presetsPage->nameInput->Disable();
      guideMenu->activeEditor->presetsPage->dirInput->Disable();
      guideMenu->activeEditor->presetsPage->trackInput->Disable();
      guideMenu->activeEditor->presetsPage->styleInput->Disable();

    }, EditorWindow::ID_WindowSelect);
  guideMenu->activeEditor->presetsPage->GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        event.Skip();
        static bool hasRun{false};
        if (std ::find(eventRunTrackers.begin(), eventRunTrackers.end(),
                       &hasRun) == eventRunTrackers.end())
          eventRunTrackers.push_back(&hasRun);
        if (hasRun)
          return;
        hasRun = true;
        ;

        generateNewPage(
            "Configuration - Presets and Styles",

            "Select your newly-created preset from the list, then choose a "
            "name for your preset.\n"
            "\n"
            "This name is mostly just for reference, and can be whatever you "
            "want,\n"
            "but if you're using an OLED and don't have special bitmaps for "
            "the font,\n"
            "this text will be displayed upon selecting the preset.\n"
            "\n"
            "This name should ideally be kept short, and if you want to put "
            "some of the name\n"
            "on a new line on the OLED, you can use \"\\n\" to mean \"Enter\"");
        guideMenu->activeEditor->presetsPage->nameInput->Enable();

        useButtonOnPage("Done", [&](wxCommandEvent& event) {
          EVENT_PAGE_SETUP;

          generateNewPage(
              "Configuration - Presets and Styles",

              "In \"Font Directory\", enter the name of the folder on your SD "
              "Card\n"
              "that contains the sound font associated with this preset.\n"
              "\n"
              "This folder doesn't need to be added to ProffieConfig in any "
              "way, it stays\n"
              "on the SD card that is in your Proffieboard.\n"
              "\n"
              "If you want to specify multiple folders if you, for example, "
              "have a \"common\"\n"
              "folder, you can seperate folder names with a \";\" (e.g. "
              "folderName;common)");
          guideMenu->activeEditor->presetsPage->dirInput->Enable();

          useButtonOnPage("Done", [&](wxCommandEvent& event) {
            EVENT_PAGE_SETUP;

            generateNewPage(
                "Configuration - Presets and Styles",

                "Now enter the name of the track you want to be associated "
                "with this preset.\n"
                "\n"
                "This track file can be in any of the directories you just "
                "specified, but if\n"
                "it's in a folder inside of one of those folders, for example "
                "a \"tracks\" folder\n"
                "you need to indicate that with a \"/\". (e.g. "
                "tracks/myTrack.wav)\n"
                "\n"
                "\".wav\" will automatically be appended to the track name, as "
                "even if you can't\n"
                "see this on your computer, all track files end with .wav.\n"
                "\n"
                "You can leave this empty if you'd like to use no track.\n");
            guideMenu->activeEditor->presetsPage->trackInput->Enable();

            useButtonOnPage("Done", [&](wxCommandEvent& event) {
              EVENT_PAGE_SETUP;

              generateNewPage("Configuration - Presets and Styles",

                              "Notice the up and down arrows beside the preset "
                              "list. When you have a preset\n"
                              "selected, and multiple presets in the list, "
                              "those allow you to move presets\n"
                              "up and down through the list.\n"
                              "\n"
                              "The order in which you cycle through presets on "
                              "the saber is determined by the order\n"
                              "of them here, so you may want to rearrange them "
                              "at some point.\n"
                              "\n"
                              "Choose one of your blades from the list to edit "
                              "the bladestyle for that blade.\n"
                              "\n"
                              "It's worth noting that if you have a blade with "
                              "SubBlades, then that blade will\n"
                              "show up in the list as [Blade Number]:[SubBlade "
                              "Number] (e.g. 0:0), as each SubBlade\n"
                              "gets its own style too.\n");
              guideMenu->activeEditor->presetsPage->bladeList->Enable();
            });
          });
        });
      },
      PresetsPage::ID_AddPreset);
  guideMenu->activeEditor->presetsPage->GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Presets and Styles",

                      "Great! Now you'll see what is known as a \"Blade Style\" show up in the box on the right.\n"
                      "\n"
                      "Every blade for every preset has a unique blade style to set up, and these blade styles\n"
                      "are the code that tells your blade what to do. These bladestyles can get really complex,\n"
                      "having all kinds of different effects, reacting to effects on the saber, etc.\n"
                      "\n"
                      "The style this field auto-populates with is pretty simple. It extends and retracts the saber\n"
                      "when you press the button, and it has an \"AudioFlicker\" between a couple of hues of blue\n"
                      "while it's on.\n"
                      "\n"
                      "Fett263's Style Library can be a good place to find bladestyles you can customize, and if\n"
                      "you're feeling adventurous, check out the ProffieOS Style Editor to make your own custom\n"
                      "styles! It's linked in \"Tools\"->\"Style Editor...\"\n"
                      "\n"
                      );

      guideMenu->activeEditor->presetsPage->styleInput->Enable();
      guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_StyleEditor, true);

      useButtonOnPage("Continue", [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;

        generateNewPage("Configuration - Saving",

                        "That's all, so next up we want to actually use the config!\n"
                        "\n"
                        "When you save your configuration, the ProffieConfig pre-checker will run to ensure there\n"
                        "are no fatal errors in your current configuration.\n"
                        "\n"
                        "If there are such errors, you will be notified, and you will need to fix them before\n"
                        "you can save your config.\n"
                        "\n"
                        "Press CTRL+S or go to \"File\"->\"Save Config\" to save.\n"
                        );

        guideMenu->activeEditor->Bind(wxEVT_MENU, [&](wxCommandEvent& event) {
            EVENT_PAGE_SETUP;

            generateNewPage("Configuration - Verification",

                            "Now, although your config may have saved successfully and passed the pre-checks,\n"
                            "it doesn't necessarily mean it's valid and will actually work.\n"
                            "\n"
                            "In order to properly confirm your config is valid, you'll want to verify it.\n"
                            "\n"
                            "Select \"Verify Config\" from the \"File\" menu to continue.\n"
                            );
            guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_VerifyConfig, true);

          }, EditorWindow::ID_SaveConfig);
      });
    }, PresetsPage::ID_BladeList);
  guideMenu->activeEditor->Bind(wxEVT_MENU, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Export Config",

                      "At some point it may be necessary to export your configuration file from ProffieConfig.\n"
                      "\n"
                      "Whether you need to share it with me if you encounter an issue, share it with someone\n"
                      "else for general help or troubleshooting, or just want to make a backup, the ability\n"
                      "to export your config will come in handy.\n"
                      "\n"
                      "When you export your config, it will still be in ProffieConfig, and editing it won't\n"
                      "affect the version in ProffieConfig, but it allows you to share it, or just look at\n"
                      "it yourself if you're curious.\n"
                      "\n"
                      "In order to export your config, select \"Export Config...\" from the \"File\" menu."
                      );
      guideMenu->activeEditor->GetMenuBar()->Enable(EditorWindow::ID_ExportConfig, true);

    }, EditorWindow::ID_VerifyConfig);
  guideMenu->activeEditor->Bind(wxEVT_MENU, [&](wxCommandEvent& event) {
      EVENT_PAGE_SETUP;

      generateNewPage("Configuration - Finishing Up",

                      "We've ran through all the basics of the ProffieConfig editor, and you can now close\n"
                      "it to continue with the rest of the introduction.\n"
                      );
      doneWithEditor = true;

      guideMenu->activeEditor->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        EVENT_PAGE_SETUP;

        generateNewPage("Finishing Up",

                        "Once you've created/edited your configuration, the last thing to do is apply\n"
                        "it to your Proffieboard.\n"
                        "\n"
                        "You can do that by pressing \"Refresh Boards\", selecting your board from the list,\n"
                        "and then clicking \"Apply Selected Configuration to Board\".\n"
                        "\n"
#                       ifdef __WINDOWS__
                        "The board name will be something like \"COM\" and then a number, but chances are\n"
                        "there'll only be the one you want, and in the event something goes wrong while\n"
                        "applying changes, you can select \"BOOTLOADER RECOVERY\" and go from there.\n"
#                       elif defined(__WXOSX__)
                          "The board name will be something like \"/dev/cu.\" and then a whole bunch of\n"
                          "letters and numbers, but chances are there'll only be the one you want.\n"
#                       elif defined(__WXGTK__)
                          "The board will be named something like \"/dev/tty\" and some more letters and\n"
                          "numbers, but chances are there'll only be the one you want.\n"
#                       endif
                        "\n"
                        "That covers the basics of ProffieConfig. There's still a few things for you to\n"
                        "discover, but that should put you well on your way! Go ahead and click \"Finish\",\n"
                        "and we'll launch the full version for you to use!\n"
                        "\n"
                        "The config you started in this introduction will still be there, and you can return\n"
                        "here any time from the menu bar under File->Re-Run First-Time Setup.\n"
                        );
        isDone = true;
        Onboard::instance->update();
      });

    }, EditorWindow::ID_ExportConfig);
}

void Onboard::Overview::generateNewPage(const std::string& headerText, const std::string& bodyText) {
  sizer->Clear(true);

  auto header = createHeader(this, headerText);
  auto body = new wxStaticText(this, wxID_ANY, bodyText, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

  sizer->Add(header);
  sizer->AddSpacer(20);
  sizer->Add(body, wxSizerFlags(1));

  SetSizerAndFit(sizer);
  Layout();
  static_cast<Onboard*>(GetParent())->Fit();
  static_cast<Onboard*>(GetParent())->Layout();
}

void Onboard::Overview::useButtonOnPage(const std::string& buttonText, std::function<void(wxCommandEvent&)> eventFunction) {
  static decltype(eventFunction) evtFunc = nullptr;
  auto* button = new wxButton(this, ID_PageButton, buttonText);
  sizer->Add(button);

  SetSizerAndFit(sizer);
  Layout();
  static_cast<Onboard*>(GetParent())->Fit();
  static_cast<Onboard*>(GetParent())->Layout();

  if (evtFunc) Unbind(wxEVT_BUTTON, evtFunc, ID_PageButton);
  evtFunc = eventFunction;
  Bind(wxEVT_BUTTON, evtFunc, ID_PageButton);
}
