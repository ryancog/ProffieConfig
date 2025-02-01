// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#pragma once

#include <unordered_map>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/panel.h>

#include "mainmenu/mainmenu.h"
#include "editor/pages/bladespage.h"

namespace Onboard {

class Welcome;
class DependencyInstall;
class Overview;
class UpdateEvent;

} // namespace Onboard

class OnboardFrame : public wxFrame {
public:
    static OnboardFrame* instance;
    OnboardFrame();
    ~OnboardFrame();

    static wxStaticText* createHeader(wxWindow*, const wxString&);

private:
    friend Onboard::Welcome;
    friend Onboard::DependencyInstall;
    friend Onboard::Overview;

    static wxEventTypeTag<Onboard::UpdateEvent> EVT_UPDATE;

    void update();
    void bindEvents();
    void dependencyInstall(wxCommandEvent&);

    wxButton* next{nullptr};
    wxButton* cancel{nullptr};
    wxButton* skipIntro{nullptr};
    wxButton* skipInstall{nullptr};

    Onboard::Welcome* welcomePage{nullptr};
    Onboard::DependencyInstall* dependencyPage{nullptr};
    Onboard::Overview* overviewPage{nullptr};

    enum {
        ID_Welcome,
        ID_DependencyInstall,
        ID_Overview,

        ID_Back,
        ID_Next,
        ID_Cancel,
        ID_SkipIntro,
        ID_SkipInstall,

        ID_PageButton,
    };
};

class Onboard::UpdateEvent : public wxCommandEvent {
public:
    UpdateEvent(wxEventTypeTag<UpdateEvent> tag, int32_t id){
        this->SetEventType(tag);
        this->SetId(id);
    }

    bool succeeded;
    wxString message;
    OnboardFrame* parent;
};


class Onboard::Welcome : public wxPanel {
public:
    Welcome(wxWindow*);
};

class Onboard::DependencyInstall : public wxPanel {
public:
    DependencyInstall(wxWindow*);

    wxStaticText* description{nullptr};
    wxStaticText* pressNext{nullptr};
    wxStaticText* doneMessage{nullptr};
    wxGauge* loadingBar{nullptr};
    wxTimer* barPulser{nullptr};
    bool completedInstall{false};
};

class Onboard::Overview : public wxPanel {
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
