#include "onboard/onboard.h"

#include "mainmenu/mainmenu.h"
#include "editor/editorwindow.h"
#include "editor/pages/propspage.h"

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

  generateNewPage("The Main Menu",

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
  guideMenu->Close(true);
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
  guideMenu->GetMenuBar()->FindWindowById(EditorWindow::ID_ExportConfig)->Disable();
  guideMenu->GetMenuBar()->FindWindowById(EditorWindow::ID_StyleEditor)->Disable();
  guideMenu->GetMenuBar()->FindWindowById(EditorWindow::ID_VerifyConfig)->Disable();
  guideMenu->activeEditor->Bind(wxEVT_UPDATE_UI, [&](wxUpdateUIEvent& event) {
    event.Skip();
    for (const auto& [ id, disabled ] : editorDisables) {
      guideMenu->activeEditor->FindWindowById(id)->Enable(!disabled);
    }
  });
}

void Onboard::Overview::linkMainMenuEvents() {
  guideMenu->Bind(wxEVT_BUTTON, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;
        generateNewPage("Adding a Configuration",

                        "Now, from this new window, you can either create a new config, or import one you already have!\n"
                        "\n"
                        "Your configurations must have unique names, but later you can import as many as you'd like.\n"
                        "\n"
                        "If you bought your saber from someone else, they should have provided you a configuration.\n"
                        "\n"
                        "These files end with \".h\" (this may not show up depending on your computer settings),\n"
                        "and contain all the information needed to make your Proffieboard work!\n"
                        );

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
        mainMenuDisables[MainMenu::ID_AddConfig] = true;
        mainMenuDisables[MainMenu::ID_EditConfig] = false;

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

      }, MainMenu::ID_EditConfig);
}

void Onboard::Overview::linkEditorEvents() {
  guideMenu->activeEditor->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent& event) {
        EVENT_PAGE_SETUP;

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
        guideMenu->activeEditor->windowSelect->Append("Blade Arrays");

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
      }, PropsPage::ID_Buttons);
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
