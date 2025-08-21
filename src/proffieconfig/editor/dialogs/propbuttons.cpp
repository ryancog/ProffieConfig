#include "propbuttons.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/propbuttons.cpp
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

#include "ui/static_box.h"

PropButtonsDialog::PropButtonsDialog(EditorWindow *parent) {
    auto& config{parent->getOpenConfig()};
    auto& prop{config.prop(config.propSelection)};

    Create(
        parent,
        wxID_ANY,
        prop.name + _(" Buttons"),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER 
    );

    auto *textSizer{new wxBoxSizer(wxVERTICAL)};

    auto propButtons{prop.buttons(config.settings.numButtons)};

    if (propButtons.empty()) {
        textSizer->Add(
            new wxStaticText(
                this,
                wxID_ANY,
                _("Selected number of buttons not supported by prop file.")
            ),
            wxSizerFlags{}.Border(wxALL, 10)
        );
    } else for (auto& [ stateName, stateButtons ] : propButtons) {
        auto *stateSizer{new PCUI::StaticBox(
            wxVERTICAL,
            this,
            wxString::Format(_("Button controls while saber is %s:"), stateName)
        )};
        auto *controlSizer{new wxBoxSizer(wxHORIZONTAL)};
        auto *buttonSizer{new wxBoxSizer(wxVERTICAL)};
        auto *actionSizer{new wxBoxSizer(wxVERTICAL)};

        // Must use Spacer, not \t, which caused rendering issues for Windows
        controlSizer->AddSpacer(50);
        controlSizer->Add(buttonSizer);
        controlSizer->Add(actionSizer);
        stateSizer->Add(controlSizer);

        for (const auto& button : stateButtons) {
            string activePredicate;
            for (const auto& [ predicate, description ]: button.descriptions) {
                auto setting = prop.dataMap().find(predicate);
                if (setting == prop.dataMap().end()) continue;

                if (setting->second->isActive()) {
                    activePredicate = setting->first;
                    break;
                }
            }

            auto key = button.descriptions.find(activePredicate);
            if (key != button.descriptions.end() && key->second != "DISABLED") {
                buttonSizer->Add(new wxStaticText(
                    stateSizer,
                    wxID_ANY,
                    button.name
                ));
                actionSizer->Add(new wxStaticText(
                    stateSizer,
                    wxID_ANY,
                    " - " + key->second
                ));
            }
        }

        if (actionSizer->IsEmpty()) {
            stateSizer->Destroy();
            delete stateSizer;
            continue;
        }
        textSizer->Add(
            stateSizer,
            wxSizerFlags(0)
                .Border(wxTOP | wxLEFT | wxRIGHT, 10).Expand()
        );
    }
    textSizer->AddSpacer(10);

    SetSizerAndFit(textSizer);
    DoLayoutAdaptation();
}

