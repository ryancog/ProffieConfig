#include "propspage.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/propspage.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

#include "../editorwindow.h"
#include "../dialogs/propbuttons.h"

PropsPage::PropsPage(EditorWindow *parent) : 
    wxPanel(parent),
    mParent{parent} {
    NotifyReceiver::create(this, mParent->getOpenConfig().propNotifyData);
    auto& config{mParent->getOpenConfig()};

    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *topSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *propSelection {new PCUI::Choice(
        this,
        config.propSelection,
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
    mPropsWindow->SetScrollbars(10, 10, -1, 1);

    sizer->Add(topSizer);
    sizer->AddSpacer(10);
    sizer->Add(mPropsWindow, wxSizerFlags(1).Expand());

    bindEvents();
    initializeNotifier();
    SetSizerAndFit(sizer);
}

void PropsPage::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        PropButtonsDialog(mParent).ShowModal();
    }, ID_Buttons);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        auto& prop{config.prop(config.propSelection)};

        wxDialog infoDialog{
            mParent,
            wxID_ANY,
            wxString::Format(_("%s Prop Info"), prop.name),
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP
        };
        auto *textSizer{new wxBoxSizer(wxVERTICAL)};
        textSizer->Add(
            infoDialog.CreateTextSizer(prop.info),
            wxSizerFlags(0).Border(wxALL, 10)
        );
        infoDialog.SetSizer(textSizer);
        infoDialog.DoLayoutAdaptation();
        infoDialog.ShowModal();
    }, ID_PropInfo);
}

void PropsPage::handleNotification(uint32 id) {
    if (id == Config::Config::ID_PROPSELECTION) {
        showSelectedProp();
    }
    if (id == ID_REBOUND or id == Config::Config::ID_PROPUPDATE) {
        loadProps();
    }
}

void PropsPage::showSelectedProp() {
    auto& config{mParent->getOpenConfig()};
    bool hasValidSelection{config.propSelection != -1 and config.propSelection < config.props().size()};
    FindWindow(ID_Buttons)->Enable(hasValidSelection);
    FindWindow(ID_PropInfo)->Enable(hasValidSelection);

    for (auto *prop : mProps) prop->ShowItems(false);
    mPropsWindow->SetSizer(nullptr, false);
    if (not hasValidSelection) return;

    auto *sizer{mProps[config.propSelection]};
    sizer->ShowItems(true);
    mPropsWindow->SetSizerAndFit(sizer);

    Layout();
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
    SetSizer(nullptr, false);
    for (auto *sizer : mProps) {
        sizer->Clear(true);
        delete sizer;
    }
    mProps.clear();

    const auto processChildren{[](
        const auto self,
        const Versions::PropLayout::Children& children,
        wxSizer *sizer,
        wxWindow *parent
    ) -> void {
        for (const auto& child : children) {
            if (const auto *ptr = std::get_if<Versions::PropSettingBase *>(&child)) {
                auto& setting{**ptr};
                wxWindow *windowToAdd{nullptr};
                if (Versions::PropSettingType::TOGGLE == setting.settingType) {
                    auto& toggle{static_cast<Versions::PropToggle&>(setting)};
                    auto *control{new PCUI::CheckBox(
                        parent,
                        toggle.value,
                        0,
                        toggle.name
                    )};
                    control->SetToolTip(toggle.description);
                    windowToAdd = control;
                } else if (Versions::PropSettingType::OPTION == setting.settingType) {
                    auto& option{static_cast<Versions::PropOption&>(setting)};

                    vector<string> labels;
                    labels.reserve(option.selections().size());
                    for (const auto& selection : option.selections()) {
                        labels.push_back(selection->name);
                    }

                    auto *control{new PCUI::Radios(
                        parent,
                        option.selection,
                        labels,
                        option.name
                    )};

                    control->SetToolTip(option.description);
                    for (auto idx{0}; idx < option.selections().size(); ++idx) {
                        control->SetToolTip(idx, option.selections()[idx]->description);
                    }
                    windowToAdd = control;
                } else if (Versions::PropSettingType::NUMERIC == setting.settingType) {
                    auto& numeric{static_cast<Versions::PropNumeric&>(setting)};
                    auto *control{new PCUI::Numeric(
                        parent,
                        numeric.value,
                        numeric.name
                    )};
                    control->SetToolTip(numeric.description);
                    windowToAdd = control;
                } else if (Versions::PropSettingType::DECIMAL == setting.settingType) {
                    auto& decimal{static_cast<Versions::PropDecimal&>(setting)};
                    auto *control{new PCUI::Decimal(
                        parent,
                        decimal.value,
                        decimal.name
                    )};
                    control->SetToolTip(decimal.description);
                    windowToAdd = control;
                } else {
                    assert(0);
                }

                if (windowToAdd) sizer->Add(windowToAdd, 0, wxEXPAND);
            } else if (const auto *ptr = std::get_if<Versions::PropLayout>(&child)) {
                wxSizer *newSizer{};
                if (ptr->label.empty()) {
                    newSizer = new wxBoxSizer(ptr->axis);
                    self(self, ptr->children, newSizer, parent);
                } else {
                    auto *box{new PCUI::StaticBox(ptr->axis, parent, ptr->label)};
                    newSizer = box->sizer();
                    self(self, ptr->children, newSizer, box);
                }
                sizer->Add(newSizer, wxSizerFlags().Expand());
            } else {
                assert(0);
            }
        }
    }};

    for (const auto *const prop : mParent->getOpenConfig().props()) {
        auto *sizer{mProps.emplace_back(new wxBoxSizer(prop->layout().axis))};
        // Top layout (as of writing) should never have label

        processChildren(processChildren, prop->layout().children, sizer, mPropsWindow);
    }

    showSelectedProp();
}
