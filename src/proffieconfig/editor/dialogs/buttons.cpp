#include "buttons.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/buttons.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/wrapsizer.h>

#include "ui/controls/button.h"
#include "ui/static_box.h"
#include "wx/event.h"

namespace {

constexpr auto PANEL_PADDING{3};

} // namespace

ButtonsDlg::ButtonsDlg(EditorWindow *parent) : 
    wxDialog(
        parent,
        wxID_ANY,
        _("Buttons") + " - " + static_cast<string>(parent->getOpenConfig().name),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    PCUI::NotifyReceiver(this, parent->getOpenConfig().settings.buttonNotifier),
    mParent{parent} {
    createUI();
    bindEvents();

    initializeNotifier();
}

void ButtonsDlg::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    createButtonsArea();

    sizer->Add(
        header(),
        wxSizerFlags(0).Expand().Border(wxALL, 10)
    );
    sizer->Add(
        mButtonsArea,
        wxSizerFlags(1).Expand().Border(wxALL, 10)
    );
    sizer->Add(
        info(this),
        wxSizerFlags(0).Expand().Border(wxALL, 10)
    );

    SetSizerAndFit(sizer);
}

void ButtonsDlg::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        mParent->getOpenConfig().settings.addButton();
    }, ID_AddButton);
}

wxBoxSizer *ButtonsDlg::header() {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    mAddButton = new wxButton(this, ID_AddButton, _("New Button"));
    sizer->Add(mAddButton);

    return sizer;
}

wxWindow *ButtonsDlg::info(wxWindow *parent) {
    auto *infoSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Buttons Configuration Information")
    )};

    auto *touchInfo{new wxHyperlinkCtrl(
        infoSizer->childParent(),
        wxID_ANY,
        _("Touch Button Info"),
        "https://pod.hubbe.net/hardware/touch-buttons.html"
    )};
    auto *commands{new wxHyperlinkCtrl(
        infoSizer->childParent(),
        wxID_ANY,
        _("Button Commands"),
        "https://pod.hubbe.net/tools/button-commands.html"
    )};
    auto *safePins{new wxHyperlinkCtrl(
        infoSizer->childParent(),
        wxID_ANY,
        _("Pins Safe for Pulldown Buttons"),
        "https://crucible.hubbe.net/t/button-types/5137/16?u=ryryog25"
    )};
    infoSizer->Add(
        touchInfo,
        wxSizerFlags().Border(wxLEFT | wxTOP | wxRIGHT, 10)
    );
    infoSizer->Add(
        commands,
        wxSizerFlags().Border(wxLEFT | wxRIGHT, 10)
    );
    infoSizer->Add(
        safePins,
        wxSizerFlags().Border(wxLEFT | wxBOTTOM | wxRIGHT, 10)
    );

    return infoSizer;
}

void ButtonsDlg::createButtonsArea() {
    mButtonsArea = new wxScrolledWindow(this, wxID_ANY);
    mButtonsArea->Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        mButtonsArea->SetVirtualSize(
            mButtonsArea->GetSize().x,
            mButtonsArea->GetVirtualSize().y
        );
        evt.Skip();
    });
    auto *sizer{new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES)};

    mButtonsArea->SetScrollRate(-1, 10);
    mButtonsArea->SetSizerAndFit(sizer);

    { 
        Config::Settings::ButtonData dummy;
        auto *dummyPanel{new ButtonPanel(mButtonsArea, mParent->getOpenConfig(), dummy)};
        mButtonsArea->SetMinSize({
            dummyPanel->GetBestSize().x + (PANEL_PADDING * 2),
            300
        });
        dummyPanel->Destroy();
    }

}

void ButtonsDlg::handleNotification(uint32) {
    mButtonsArea->GetSizer()->Clear(true);

    auto& config{mParent->getOpenConfig()};
    const auto& numButtons{config.settings.numButtons()};
    if (numButtons == 0) {
        mButtonsArea->GetSizer()->AddStretchSpacer();
        mButtonsArea->GetSizer()->Add(
            new wxStaticText(
                mButtonsArea,
                wxID_ANY,
                _("This config has no buttons."),
                wxDefaultPosition,
                wxDefaultSize,
                wxALIGN_CENTER_HORIZONTAL
            ),
            wxSizerFlags(0).Expand()
        );
        mButtonsArea->GetSizer()->AddStretchSpacer();
    } else {
        for (auto idx{0}; idx < numButtons; ++idx) {
            auto& button{config.settings.button(idx)};
            mButtonsArea->GetSizer()->Add(
                new ButtonPanel(mButtonsArea, config, *button),
                wxSizerFlags(0).Border(wxALL, PANEL_PADDING)
            );
        }
    }

    Layout();
}


ButtonsDlg::ButtonPanel::ButtonPanel(
    wxScrolledWindow *parent,
    Config::Config& config,
    Config::Settings::ButtonData& button
) : PCUI::StaticBox(wxHORIZONTAL, parent) {
    auto *type{new PCUI::Choice(
        childParent(),
        button.type,
        _("Type"),
        wxHORIZONTAL
    )};
    type->SetToolTip(_(
        "The physical kind of button wired to the board.\n"
        "Pullup means the switch is wired to its pin and GND.\n"
        "Pulldown means the switch is wired to its pin and 3v3 or BATT+\n"
        " - If you're using a pulldown button, be sure the pin is tolerant! See link."
    ));

    auto *event{new PCUI::Choice(
        childParent(),
        button.event,
        _("Event"),
        wxHORIZONTAL
    )};
    event->SetToolTip(_("The event a button press triggers."));

    auto *remove{new PCUI::Button(
        childParent(),
        wxID_ANY,
        _("Remove")
    )};
    remove->SetToolTip(_("Remove this button"));

    auto *column1{new wxBoxSizer{wxVERTICAL}};
    column1->Add(type, 0, wxEXPAND);
    column1->AddSpacer(10);
    column1->Add(event, 0, wxEXPAND);
    column1->AddSpacer(10);
    column1->Add(remove);

    auto *pin{new PCUI::ComboBox(
        childParent(),
        button.pin,
        _("Pin"),
        wxHORIZONTAL
    )};
    pin->SetToolTip(_("The pin on the board the button is wired to."));

    auto *name{new PCUI::Text(
        childParent(),
        button.name,
        0,
        false,
        _("Name"),
        wxHORIZONTAL
    )};
    name->SetToolTip(_("The button name for Serial Monitor and debugging."));

    auto *touch{new PCUI::Numeric(
        childParent(),
        button.touch,
        _("Threshold"),
        wxHORIZONTAL
    )};
    touch->SetToolTip(_("Touch threshold. See the link for more information below."));

    auto *column2{new wxBoxSizer{wxVERTICAL}};
    column2->Add(pin, 0, wxEXPAND);
    column2->AddSpacer(10);
    column2->Add(name, 0, wxEXPAND);
    column2->AddSpacer(10);
    column2->Add(touch, 0, wxEXPAND);

    Add(column1, 0);
    AddSpacer(10);
    Add(column2, 0);

    remove->Bind(wxEVT_BUTTON, [&config, &button](wxCommandEvent&) {
        config.settings.removeButton(button);
    });

    Layout();
    Fit();
}

