#include "propspage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

#include "../../core/defines.h"
#include "../editorwindow.h"
#include "../dialogs/propbuttons.h"

PropsPage::PropsPage(EditorWindow *parent) : 
    wxPanel(parent),
    mParent{parent} {
    Notifier::create(this, mNotifyData);
    auto config{mParent->getOpenConfig()};

    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *topSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *propSelection {new PCUI::Choice(
        this,
        config->propSelection,
        _("Prop File")
    )};
    propSelection->SetMinSize(wxSize{120, -1});
    auto *propInfo{new wxButton(
        this,
        ID_PropInfo,
        _("Prop Description and Usage Info...")
    )};
    auto *buttonInfo{new wxButton(
        this,
        ID_Buttons,
        _("Button Controls...")
    )};
    propInfo->SetToolTip(_("View prop creator-provided information about this prop and its intended usage."));
    buttonInfo->SetToolTip(_("View button controls based on specific option settings and number of buttons."));
    topSizer->Add(propSelection, wxSizerFlags());
    topSizer->AddSpacer(5);
    topSizer->Add(propInfo, wxSizerFlags().Bottom());
    topSizer->AddSpacer(5);
    topSizer->Add(buttonInfo, wxSizerFlags().Bottom());

    mPropsWindow = new wxScrolledWindow(this, wxID_ANY);
    auto *propsSizer{new wxBoxSizer(wxVERTICAL)};
    mPropsWindow->SetSizerAndFit(propsSizer);
    mPropsWindow->SetScrollbars(10, 10, -1, 1);

    sizer->Add(topSizer);
    sizer->Add(mPropsWindow, wxSizerFlags(1).Expand());

    loadProps();
    bindEvents();
    initializeNotifier();
    SetSizerAndFit(sizer);
}

void PropsPage::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        PropButtonsDialog(mParent).ShowModal();
    }, ID_Buttons);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto config{mParent->getOpenConfig()};
        auto prop{config->prop(config->propSelection)};

        wxDialog infoDialog{
            mParent,
            wxID_ANY,
            wxString::Format(_("%s Prop Info"), prop->name),
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP
        };
        auto *textSizer{new wxBoxSizer(wxVERTICAL)};
        textSizer->Add(
            infoDialog.CreateTextSizer(prop->info),
            wxSizerFlags(0).Border(wxALL, 10)
        );
        infoDialog.SetSizer(textSizer);
        infoDialog.DoLayoutAdaptation();
        infoDialog.ShowModal();
    }, ID_PropInfo);
}

void PropsPage::handleNotification(uint32 id) {
    if (id == ID_REBOUND or id == ID_PropSelection) {
        auto config{mParent->getOpenConfig()};
        FindWindow(ID_Buttons)->Enable(config->propSelection != -1);
        FindWindow(ID_PropInfo)->Enable(config->propSelection != -1);
    }
}

// void PropsPage::update() {
//     if (mPropsWindow->IsShown()) {
//         mPropsWindow->FitInside();
//         mPropsWindow->GetSizer()->Layout();
//         mPropsWindow->GetSizer()->Fit(mPropsWindow);
// 
//         mPropsWindow->InvalidateBestSize();
//         auto propsWindowBestSize{mPropsWindow->GetBestVirtualSize()};
//         mPropsWindow->SetSizeHints(propsWindowBestSize);
// 
//         GetStaticBox()->Layout();
//         Fit(GetStaticBox());
// 
//         auto windowSelectSize{mParent->windowSelect->GetSize()};
//         auto topSize{mTopSizer->GetSize()};
//         mParent->SetMinSize(wxSize{
//             mTopSizer->GetSize().x + 60,
//             windowSelectSize.y + topSize.y + 200,
//         });
//         mParent->Layout();
//         mParent->Fit();
//     }
// }

void PropsPage::loadProps() {
//     mProps.clear();
// 
//     auto addProp{[&](const string_view& propName, bool builtin = false) {
//         auto *propConfig{PropFile::createPropConfig(string{propName}, propsWindow, builtin)};
//         if (propConfig != nullptr) {
//             mPropsWindow->GetSizer()->Add(propConfig);
//             mProps.push_back(propConfig);
//         }
//     }};
// 
//     for (const auto& prop : AppState::BUILTIN_PROPS) {
//         addProp(prop, true);
//     }
//     for (const auto& prop : AppState::getPropFileNames()) {
//         addProp(prop);
//     }
//     updateProps();
// 
//     Layout();
}
