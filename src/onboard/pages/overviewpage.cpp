#include "mainmenu/dialogs/addconfig.h"
#include "onboard/onboard.h"

#include "mainmenu/mainmenu.h"
#include "editor/editorwindow.h"
#include "editor/pages/propspage.h"
#include "editor/pages/bladespage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/bladearraypage.h"

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>

#define EVENT_PAGE_SETUP \
event.Skip(); \
    static bool hasRun{false}; \
    if (hasRun) return; \
    hasRun = true;

Onboard::Overview::Overview(wxWindow* parent) : wxWindow(parent, ID_Overview) {
  sizer = new wxBoxSizer(wxVERTICAL);

  generateNewPage("Introduction to ProffieConfig",

                  "This is ProffieConfig's main menu.\n"
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
  prepareMainMenu();
  linkMainMenuEvents();
}
void Onboard::Overview::prepareMainMenu() {
  guideMenu->GetMenuBar()->Disable();
  guideMenu->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    if (event.CanVeto()) {
      event.Veto();
      wxMessageBox("You cannot close this during First-Time Setup.", "Close ProffieConfig", wxOK | wxCENTER, guideMenu);
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
                        "These files end with \".h\" if you're looking for them on your computer to import*,"
                        "and contain all the information needed to make your Proffieboard work!\n"
                        "\n"
                        "*This may not show up depending on your computer settings!\n"
                        );
        auto configDialog = AddConfig(guideMenu);
        static_cast<wxToggleButton*>(configDialog.FindWindow(AddConfig::ID_ImportExisting))->Disable();
        configDialog.ShowModal();

      }, MainMenu::ID_AddConfig);
  guideMenu->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        generateNewPage("Edit Configuration",

                        "Now that you've imported your config, ProffieConfig will manage it, and you no longer need\n"
                        "the file, though it's never a bad idea to keep backups.\n"
                        "\n"
                        "We'll go over how to export your configuration later, which could also be useful if\n"
                        "you ever need help troubleshooting.\n"
                        "\n\n"
                        "Click on \"Edit Selected Configuration\" in order to open your config\n"
                        "in the ProffieConfig editor\n"
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

        guideMenu->activeEditor->windowSelect->Clear();
        guideMenu->activeEditor->windowSelect->Append("General");
        guideMenu->activeEditor->windowSelect->Append("Prop File");
        guideMenu->activeEditor->windowSelect->SetSelection(0);

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
  guideMenu->activeEditor->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        if (guideMenu->activeEditor->windowSelect->GetSelection() != 1) {
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

      }, EditorWindow::ID_WindowSelect);
  guideMenu->activeEditor->propsPage->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
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

      }, PropsPage::ID_PropSelect);
  guideMenu->activeEditor->propsPage->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
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
        guideMenu->activeEditor->windowSelect->Append("Blade Arrays");

      }, PropsPage::ID_Buttons);
  guideMenu->activeEditor->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        if (guideMenu->activeEditor->windowSelect->GetSelection() != 2) {
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
                        "uses the \"GRB\". If you're unsure, simply leave the color order as it is.\n"
                        "\n"
                        "The data pin is the physical pin your blade is connected to on the Proffieboard for\n"
                        "sending the, well, data. You can select an option from the drop-down, or type directly\n"
                        "in the box to specify any other (supported) pin on the Proffieboard.\n"
                        "\n"
                        "Once you're familiar with these settings, click the \"+\" icon under the \"SubBlades\"\n"
                        "list and select the created SubBlade (\"SubBlade 0\").\n"
                        );

      }, BladesPage::ID_BladeSelect);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;

        generateNewPage("Configuration - Blade Arrays",

                        "Now, you'll notice mostly the same controls, but with a few extra specific\n"
                        "to WS281X blades in SubBlade mode.\n"
                        "\n"
                        "The most important bit here is the \"SubBlade Start\" and \"SubBlade End\",\n"
                        "which specifies the subsection of WS281X blade to be considered a different blade.\n"
                        "\n"
                        "The range for these numbers start at 0 and end at \"Number of Pixels\", and\n"
                        "the end of one SubBlade should not overlap with the start of another."
                        "\n"
                        "These SubBlades don't have to be in order (e.g. the physical end of a WS281X\n"
                        "blade could be the first SubBlade)\n"
                        "\n"
                        "When SubBlade mode is active, only SubBlade 0 will show blade controls, the\n"
                        "rest will just be the settings for the specific SubBlade.\n"
                        "\n"
                        "Now, try changing the type of Blade to a \"Tri-LED Star\"\n"
                        );
        bladeDisables.at(BladesPage::ID_BladeType) = false;
        guideMenu->activeEditor->bladesPage->bladeType->Clear();
        guideMenu->activeEditor->bladesPage->bladeType->Append("WS281X (RGB)");
        guideMenu->activeEditor->bladesPage->bladeType->Append("WS281X (RGBW)");
        guideMenu->activeEditor->bladesPage->bladeType->Append("Tri-LED Star");
        guideMenu->activeEditor->bladesPage->bladeType->Append("Quad-LED Star");
        guideMenu->activeEditor->bladesPage->bladeType->SetSelection(0);

      }, BladesPage::ID_SubBladeSelect);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        if (guideMenu->activeEditor->bladesPage->bladeType->GetSelection() != 2) {
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
                        "Finally, check out the \"Single Color\" blade type.\n"
                        );
        guideMenu->activeEditor->bladesPage->bladeType->Append("Single Color");

      }, BladesPage::ID_BladeType);
  guideMenu->activeEditor->bladesPage->GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        if (guideMenu->activeEditor->bladesPage->bladeType->GetSelection() != 4) {
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
                        "Alternatively, you could just create a Tri-LED Star blade and only set up one of\n"
                        "the LEDs to achieve a similar effect.\n"
                        "\n"
                        "Now that we've covered the basics of blades, you should know you can add as many\n"
                        "blades as you'd like, provided you have the pins on the Proffieboard, of course!\n"
                        "For WS281X blades, Power Pins can be shared, which can be useful if\n"
                        "you're running low.\n"
                        "\n"
                        "Switch the page to \"Presets And Styles\" to continue.\n"
                        );
        guideMenu->activeEditor->windowSelect->Append("Presets And Styles");

      }, BladesPage::ID_BladeType);
  guideMenu->activeEditor->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        if (guideMenu->activeEditor->windowSelect->GetSelection() != 3) {
          hasRun = false;
          return;
        }

        generateNewPage("Configuration - Presets and Styles",

                        "Presets are arguably the most important part of the configuration.\n"
                        "\n"
                        "Presets contain all the information about what to actually do whenever\n"
                        "with all your LEDs and what kind of effects should happen.\n"
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
        EVENT_PAGE_SETUP;

        generateNewPage("Configuration - Presets and Styles",

                        "Select your newly-created preset from the list, then choose a name for your preset.\n"
                        "\n"
                        "This name is mostly just for reference, and can be whatever you want,\n"
                        "but if you're using an OLED and don't have special bitmaps for the font,\n"
                        "this text will be displayed upon selecting the preset.\n"
                        "\n"
                        "This name should ideally be kept short, and if you want to put some of the name\n"
                        "on a new line on the OLED, you can use \"\\n\" to mean \"Enter\""
                        );
        guideMenu->activeEditor->presetsPage->nameInput->Enable();
        guideMenu->activeEditor->presetsPage->addPreset->Disable();

        useButtonOnPage("Done", [&](wxCommandEvent&) {
          EVENT_PAGE_SETUP; // This is kind of an ugly way to do this, but I can't capture this function for unbind.

          generateNewPage("Configuration - Presets and Styles",

                          "In \"Font Directory\", enter the name of the folder on your SD Card\n"
                          "that contains the sound font associated with this preset.\n"
                          "\n"
                          "This folder doesn't need to be added to ProffieConfig in any way, it stays\n"
                          "on the SD card that is in your Proffieboard.\n"
                          "\n"
                          "If you want to specify multiple folders if you, for example, have a \"common\"\n"
                          "folder, you can seperate folder names with a \";\" (e.g. folderName;common)"
                          );
          guideMenu->activeEditor->presetsPage->dirInput->Enable();

          useButtonOnPage("Done", [&](wxCommandEvent&) {
            EVENT_PAGE_SETUP; // This is kind of an ugly way to do this, but I can't capture this function for unbind.

            generateNewPage("Configuration - Presets and Styles",

                            "Now enter the name of the track you want to be associated with this preset.\n"
                            "\n"
                            "This track file can be in any of the directories you just specified, but if\n"
                            "it's in a folder inside of one of those folders, for example a \"tracks\" folder\n"
                            "you need to indicate that with a \"/\". (e.g. tracks/myTrack.wav)\n"
                            "\n"
                            "\".wav\" will automatically be appended to the track name, as even if you can't\n"
                            "see this on your computer, all track files end with .wav.\n"
                            "\n"
                            "You can leave this empty if you'd like to use no track.\n"
                            );
            guideMenu->activeEditor->presetsPage->trackInput->Enable();

            useButtonOnPage("Done", [&](wxCommandEvent&) {
              EVENT_PAGE_SETUP; // This is kind of an ugly way to do this, but I can't capture this function for unbind.

              generateNewPage("Configuration - Presets and Styles",

                              "Choose one of your blades from the list to edit the bladestyle for that blade.\n"
                              );

              guideMenu->activeEditor->presetsPage->bladeList->Enable();
            });
          });
        });

      }, PresetsPage::ID_AddPreset);
}

void Onboard::Overview::generateNewPage(const std::string& headerText, const std::string& bodyText) {
  sizer->Clear(true);

  auto header = createHeader(this, headerText);
  auto body = new wxStaticText(this, wxID_ANY, bodyText, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

  sizer->Add(header);
  sizer->AddSpacer(20);
  sizer->Add(body, wxSizerFlags(1));

  Layout();
  Fit();
}

void Onboard::Overview::useButtonOnPage(const std::string& buttonText, std::function<void(wxCommandEvent&)> eventFunction) {
  auto button = new wxButton(this, ID_PageButton, buttonText);
  sizer->Add(button);

  Bind(wxEVT_BUTTON, eventFunction, ID_PageButton);
}
