#include "propspage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

#include "ui/controls.h"

#include "../../core/defines.h"
#include "../../core/appstate.h"
#include "../../core/utilities/misc.h"
#include "../editorwindow.h"
#include "generalpage.h"

PropsPage::PropsPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window), mParent{static_cast<EditorWindow*>(window)} {
    mTopSizer = new wxBoxSizer(wxHORIZONTAL);
    propSelection = new PCUI::Choice(GetStaticBox(), ID_PropSelect, Misc::createEntries({_("Default")}), _("Prop File"));
    propSelection->SetMinSize(wxSize{120, -1});
    // Two ampersands bc wxWidgets formatting
    propInfo = new wxButton(GetStaticBox(), ID_PropInfo, _("Prop Description and Usage Info..."));
    buttonInfo = new wxButton(GetStaticBox(), ID_Buttons, _("Button Controls..."));
    TIP(propInfo, _("View prop creator-provided information about this prop and its intended usage."));
    TIP(buttonInfo, _("View button controls based on specific option settings and number of buttons."));
    mTopSizer->Add(propSelection, wxSizerFlags(0).Border(wxALL, 10));
    mTopSizer->Add(propInfo, wxSizerFlags(0).Border(wxALL, 10).Bottom());
    mTopSizer->Add(buttonInfo, wxSizerFlags(0).Border(wxALL, 10).Bottom());

    propsWindow = new wxScrolledWindow(GetStaticBox(), wxID_ANY);
    auto *propsSizer{new wxBoxSizer(wxVERTICAL)};
    propsWindow->SetSizerAndFit(propsSizer);
    propsWindow->SetScrollbars(10, 10, -1, 1);

    Add(mTopSizer);
    Add(propsWindow, wxSizerFlags(1).Expand());
    loadProps();
    bindEvents();
}

void PropsPage::bindEvents() {
    auto propSelectUpdate = [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        updateSelectedProp();
        update();

        wxSetCursor(wxNullCursor);
    };
    auto optionSelectUpdate = [&](wxCommandEvent&) {
        int32 x{0};
        int32 y{0};
        propsWindow->GetViewStart(&x, &y);
        update();
        propsWindow->Scroll(x, y);
        mParent->Layout();
    };

    GetStaticBox()->Bind(wxEVT_CHOICE, propSelectUpdate, ID_PropSelect);
    propsWindow->Bind(wxEVT_CHECKBOX, optionSelectUpdate, wxID_ANY);
    propsWindow->Bind(wxEVT_RADIOBUTTON, optionSelectUpdate, wxID_ANY);
    propsWindow->Bind(wxEVT_SPINCTRL, optionSelectUpdate, wxID_ANY);
    propsWindow->Bind(wxEVT_SPINCTRLDOUBLE, optionSelectUpdate, wxID_ANY);

    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        PropFile* activeProp{nullptr};
        for (auto& prop : mProps) {
            if (propSelection->entry()->GetStringSelection() == prop->getName()) activeProp = prop;
        }

        auto *textSizer{new wxBoxSizer(wxVERTICAL)};
        wxDialog buttonDialog{
            mParent,
            wxID_ANY,
            (activeProp ? activeProp->getName() : _("Default")) + _(" Buttons"),
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER
        };

        if (activeProp == nullptr) {
            auto defaultButtons{[](int32 num) -> wxString {
                auto zeroButtons{[]() {
                    return _(
                            "On/Off - Twist\n"
                            "Next preset - Point up and shake\n"
                            "Clash - Hit the blade while saber is on.");
                }};
                auto oneButton{[]() {
                    return _(
                            "On/Off - Click to turn the saber on or off.\n"
                            "Turn On muted - Double-click\n"
                            "Next preset - Hold button and hit the blade while saber is off.\n"
                            "Clash - Hit the blade while saber is on.\n"
                            "Lockup - Hold button, then trigger a clash. Release button to end.\n"
                            "Drag - Hold button, then trigger a clash while pointing down. Release button to end.\n"
                            "Melt - Hold button and stab something.\n"
                            "Force - Long-click button.\n"
                            "Start Soundtrack - Long-click the button while blade is off.\n"
                            "Enter/Exit Color Change - Hold button and Twist.");
                }};
                auto twoButton{[]() {
                    return _(
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
                            "Enter/Exit Color Change - Hold Aux and click POW while on.");
                }};

                switch (num) {
                    case 0: 
                        return zeroButtons();
                    case 1: 
                        return oneButton();
                    case 2: 
                    case 3: 
                        return twoButton();
                    default:
                        return _("Button Configuration Not Supported");
                }
            }};
            textSizer->Add(
                new wxStaticText(&buttonDialog, wxID_ANY, defaultButtons(mParent->generalPage->buttons->entry()->GetValue())),
                wxSizerFlags{}.Border(wxALL, 10)
            );
        } else {
            auto propButtons = activeProp->getButtons()[mParent->generalPage->buttons->entry()->GetValue()];

            if (propButtons.empty()) {
                textSizer->Add(
                    new wxStaticText(&buttonDialog, wxID_ANY, _("Selected number of buttons not supported by prop file.")),
                    wxSizerFlags{}.Border(wxALL, 10)
                );
            } else for (auto& [ stateName, stateButtons ] : propButtons) {
                auto *stateSizer{new wxStaticBoxSizer(wxVERTICAL, &buttonDialog, wxString::Format(_("Button controls while saber is %s:"), stateName))};
                auto *controlSizer{new wxBoxSizer(wxHORIZONTAL)};
                auto *buttonSizer{new wxBoxSizer(wxVERTICAL)};
                auto *actionSizer{new wxBoxSizer(wxVERTICAL)};

                // Must use Spacer, not \t, which caused rendering issues for Windows
                controlSizer->AddSpacer(50);
                controlSizer->Add(buttonSizer);
                controlSizer->Add(actionSizer);
                stateSizer->Add(controlSizer);

                for (auto& button : stateButtons) {
                    string activePredicate;
                    for (const auto& [ predicate, description ]: button.descriptions) {
                        auto setting = activeProp->getSettings()->find(predicate);
                        if (setting == activeProp->getSettings()->end()) continue;

                        if (not setting->second.getOutput().empty()) {
                            activePredicate = setting->first;
                            break;
                        }
                    }

                    auto key = button.descriptions.find(activePredicate);
                    if (key != button.descriptions.end() && key->second != "DISABLED") {
                        buttonSizer->Add(new wxStaticText(stateSizer->GetStaticBox(), wxID_ANY, button.name));
                        actionSizer->Add(new wxStaticText(stateSizer->GetStaticBox(), wxID_ANY, " - " + key->second));
                    }
                }

                if (actionSizer->IsEmpty()) {
                    stateSizer->GetStaticBox()->Destroy();
                    delete stateSizer;
                    continue;
                }
                textSizer->Add(stateSizer, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).Expand());
            }
            textSizer->AddSpacer(10);
        }

        buttonDialog.SetSizerAndFit(textSizer);
        buttonDialog.DoLayoutAdaptation();
        wxSetCursor(wxNullCursor);
        buttonDialog.ShowModal();
    }, ID_Buttons);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        wxString info;

        PropFile* activeProp{nullptr};
        for (auto& prop : mProps) {
            if (propSelection->entry()->GetStringSelection() == prop->getName()) activeProp = prop;
        }

        if (activeProp == nullptr) {
            info = _("The default ProffieOS prop file.");
        } else {
            info = activeProp->getInfo();
        }

        wxDialog infoDialog{
            mParent,
            wxID_ANY,
            wxString::Format(_("%s Prop Info"), propSelection->entry()->GetStringSelection()),
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP
        };
        auto *textSizer{new wxBoxSizer(wxVERTICAL)};
        textSizer->Add(infoDialog.CreateTextSizer(info), wxSizerFlags(0).Border(wxALL, 10));
        infoDialog.SetSizer(textSizer);
        infoDialog.DoLayoutAdaptation();
        wxSetCursor(wxNullCursor);
        infoDialog.ShowModal();
    }, ID_PropInfo);
}

void PropsPage::updateProps() {
    auto lastSelect{propSelection->entry()->GetStringSelection()};
    propSelection->entry()->Clear();
    propSelection->entry()->Append(_("Default"));

    for (const auto& prop : mProps) {
        propSelection->entry()->Append(prop->getName());
    }

    if ([this, lastSelect]() { for (const auto& prop : propSelection->entry()->GetStrings()) if (prop == lastSelect) return true; return false; }()) {
        propSelection->entry()->SetStringSelection(lastSelect);
    } else propSelection->entry()->SetSelection(0);
}

void PropsPage::update() {
    for (auto& prop : mProps) {
        prop->SetSize(0, 0);
        if (propSelection->entry()->GetStringSelection() != prop->getName()) continue;

        updateDisables(prop);

        for (auto& [ name, setting ] : *prop->getSettings()) {
            setting.enable(!setting.disabled && setting.checkRequiredSatisfied(*prop->getSettings()));
        }
    }

    if (propsWindow->IsShown()) {
        propsWindow->FitInside();
        propsWindow->GetSizer()->Layout();
        propsWindow->GetSizer()->Fit(propsWindow);

        propsWindow->InvalidateBestSize();
        auto propsWindowBestSize{propsWindow->GetBestVirtualSize()};
        propsWindow->SetSizeHints(propsWindowBestSize);

        GetStaticBox()->Layout();
        Fit(GetStaticBox());

        auto windowSelectSize{mParent->windowSelect->GetSize()};
        auto topSize{mTopSizer->GetSize()};
        mParent->SetMinSize(wxSize{
            mTopSizer->GetSize().x + 60,
            windowSelectSize.y + topSize.y + 200,
        });
        mParent->Layout();
        mParent->Fit();
    }
}

void PropsPage::updateDisables(PropFile* prop) {
    for (auto& [ name, setting ] : *prop->getSettings()) {
        setting.disabled = false;
    }
    for (auto& [ name, setting ] : *prop->getSettings()) {
        for (const auto& disable : setting.disables) {
            auto key = prop->getSettings()->find(disable);
            if (key == prop->getSettings()->end()) continue;

            key->second.disabled |= !setting.getOutput().empty();
        }
    }
}

const std::vector<PropFile*>& PropsPage::getLoadedProps() { return mProps; }

PropFile* PropsPage::getSelectedProp() {
    for (const auto& prop : mProps) {
        if (prop->getName() == propSelection->entry()->GetStringSelection()) return prop;
    }
    return nullptr;
}

void PropsPage::updateSelectedProp(const string& newProp) {
    if (!newProp.empty()) propSelection->entry()->SetStringSelection(newProp);
    for (auto& prop : mProps) {
        prop->Show(propSelection->entry()->GetStringSelection() == prop->getName());
    }
}

void PropsPage::loadProps() {
    mProps.clear();

    auto addProp{[&](const string_view& propName, bool builtin = false) {
        auto *propConfig{PropFile::createPropConfig(string{propName}, propsWindow, builtin)};
        if (propConfig != nullptr) {
            propsWindow->GetSizer()->Add(propConfig);
            mProps.push_back(propConfig);
        }
    }};

    for (const auto& prop : AppState::BUILTIN_PROPS) {
        addProp(prop, true);
    }
    for (const auto& prop : AppState::getPropFileNames()) {
        addProp(prop);
    }
    updateProps();

    Layout();
}
