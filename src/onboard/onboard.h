// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "mainmenu/mainmenu.h"

#include <unordered_map>
#include <wx/wizard.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>

class Onboard : public wxFrame {
public:
  static Onboard* instance;
  Onboard();

private:
  static wxStaticText* createHeader(wxWindow*, const wxString&);

  void update();
  void bindEvents();
  void dependencyInstall(wxCommandEvent&);

  class Welcome;
  class DependencyInstall;
  class Overview;

  wxButton* next{nullptr};
  wxButton* cancel{nullptr};

  Welcome* welcomePage{nullptr};
  DependencyInstall* dependencyPage{nullptr};
  Overview* overviewPage{nullptr};

  enum {
    ID_Welcome,
    ID_DependencyInstall,
    ID_Overview,

    ID_Back,
    ID_Next,
    ID_Cancel,
  };
};





class Onboard::Welcome : public wxWindow {
public:
  Welcome(wxWindow*);
};

class Onboard::DependencyInstall : public wxWindow {
public:
  DependencyInstall(wxWindow*);

  wxStaticText* description{nullptr};
  wxStaticText* pressNext{nullptr};
  wxStaticText* doneMessage{nullptr};
  wxGauge* loadingBar{nullptr};
  wxTimer* barPulser{nullptr};
  bool completedInstall{false};
};

class Onboard::Overview : public wxWindow {
public:
  Overview(wxWindow*);
  ~Overview();

  void prepare();

  bool isDone{false};

  std::unordered_map<int32_t, bool> mainMenuDisables{
      { MainMenu::ID_ConfigSelect, true },
      { MainMenu::ID_AddConfig, true },
      { MainMenu::ID_RemoveConfig, true },
      { MainMenu::ID_RefreshDev, true },
      { MainMenu::ID_DeviceSelect, true },
      { MainMenu::ID_ApplyChanges, true },
      { MainMenu::ID_EditConfig, true },
      { MainMenu::ID_OpenSerial, true }
  };
  std::unordered_map<int32_t, bool> editorDisables{

  };

private:
  MainMenu* guideMenu{nullptr};

  wxBoxSizer* sizer{nullptr};

  void generateNewPage(const std::string&, const std::string&);
  void prepareMainMenu();
  void prepareEditor();
  void linkMainMenuEvents();
  void linkEditorEvents();

  void updateEditorDisables();
};
