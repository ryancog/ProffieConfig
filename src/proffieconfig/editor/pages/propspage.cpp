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
    wxStaticBoxSizer(wxVERTICAL, parent),
    mParent{parent} {
    auto config{mParent->getOpenConfig()};

    auto *topSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *propSelection {new PCUI::Choice(
        GetStaticBox(),
        config->propSelection,
        _("Prop File")
    )};
    propSelection->SetMinSize(wxSize{120, -1});
    auto *propInfo{new wxButton(
        GetStaticBox(),
        ID_PropInfo,
        _("Prop Description and Usage Info...")
    )};
    auto *buttonInfo{new wxButton(
        GetStaticBox(),
        ID_Buttons,
        _("Button Controls...")
    )};
    TIP(propInfo, _("View prop creator-provided information about this prop and its intended usage."));
    TIP(buttonInfo, _("View button controls based on specific option settings and number of buttons."));
    topSizer->Add(
        propSelection,
        wxSizerFlags(0).Border(wxALL, 10)
    );
    topSizer->Add(
        propInfo,
        wxSizerFlags(0).Border(wxALL, 10).Bottom()
    );
    topSizer->Add(
        buttonInfo,
        wxSizerFlags(0).Border(wxALL, 10).Bottom()
    );

    mPropsWindow = new wxScrolledWindow(GetStaticBox(), wxID_ANY);
    auto *propsSizer{new wxBoxSizer(wxVERTICAL)};
    mPropsWindow->SetSizerAndFit(propsSizer);
    mPropsWindow->SetScrollbars(10, 10, -1, 1);

    Add(topSizer);
    Add(mPropsWindow, wxSizerFlags(1).Expand());

    loadProps();
    bindEvents();
    Notifier::create(GetStaticBox(), mNotifyData);
    mNotifyData.getLock().lock();
    mNotifyData.notify(ID_PropSelection);
    mNotifyData.getLock().unlock();
}

void PropsPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        PropButtonsDialog(mParent).ShowModal();
    }, ID_Buttons);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
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
    if (id == ID_PropSelection) {
        auto config{mParent->getOpenConfig()};
        GetStaticBox()->FindWindow(ID_Buttons)->Enable(config->propSelection != -1);
        GetStaticBox()->FindWindow(ID_PropInfo)->Enable(config->propSelection != -1);
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
