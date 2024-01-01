// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "mainmenu/mainmenu.h"
#include "editor/pages/bladespage.h"
#include "editor/pages/bladearraypage.h"

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
  wxButton* skip{nullptr};

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
    ID_Skip,

    ID_PageButton,
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

  // Used for things that like to re-enable themselves
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
  std::unordered_map<int32_t, bool> bladeDisables{
      { BladesPage::ID_BladeType, true },
      { BladesPage::ID_RemoveSubBlade, true },
      { BladesPage::ID_AddSubBlade, true }
  };
  std::unordered_map<int32_t, bool> awarenessDisables {
      { BladeArrayPage::ID_BladeIDEnable, true },
      { BladeArrayPage::ID_BladeDetectEnable, true},
      { BladeArrayPage::ID_BladeArray, true },
      { BladeArrayPage::ID_AddArray, true },
      { BladeArrayPage::ID_RemoveArray, true },
      { BladeArrayPage::ID_BladeIDPower, true },
      { BladeArrayPage::ID_ContinuousScan, true }
  };

private:
  bool doneWithEditor{false};
  static std::vector<bool*> eventRunTrackers;
  MainMenu* guideMenu{nullptr};

  wxBoxSizer* sizer{nullptr};

  void generateNewPage(const std::string&, const std::string&);
  void useButtonOnPage(const std::string&, std::function<void(wxCommandEvent&)>);
  void prepareMainMenu();
  void prepareEditor();
  void linkMainMenuEvents();
  void linkEditorEvents();

  void updateEditorDisables();
};
