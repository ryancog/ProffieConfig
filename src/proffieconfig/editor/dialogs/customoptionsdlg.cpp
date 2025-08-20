#include "customoptionsdlg.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>

#include "ui/static_box.h"

CustomOptionsDlg::CustomOptionsDlg(EditorWindow *parent) : 
    wxDialog(
        parent,
        wxID_ANY,
        _("Custom Options") + " - " + static_cast<string>(parent->getOpenConfig().name),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    PCUI::NotifyReceiver(this, parent->getOpenConfig().settings.customOptsNotifyData),
    mParent{parent} {
    createUI();
    bindEvents();

    initializeNotifier();
}

void CustomOptionsDlg::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    createOptionArea();

    sizer->Add(
        header(),
        wxSizerFlags(0).Expand().Border(wxALL, 10)
    );
    sizer->Add(
        mOptionArea,
        wxSizerFlags(1).Expand().Border(wxALL, 10)
    );
    sizer->Add(
        info(this),
        wxSizerFlags(0).Expand().Border(wxALL, 10)
    );
    sizer->SetMinSize(500, 500);

    SetSizerAndFit(sizer);
}

void CustomOptionsDlg::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        mParent->getOpenConfig().settings.addCustomOption();
    }, ID_AddDefine);
}

wxBoxSizer *CustomOptionsDlg::header() {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *text{new wxStaticText(
        this,
        wxID_ANY, 
        _("Defines are in the format:\n#define [NAME] [VALUE]")
    )};
    mAddDefineButton = new wxButton(this, ID_AddDefine, _("Add Custom Define"));
    sizer->Add(text, wxSizerFlags(0).Center());
    sizer->AddSpacer(50);
    sizer->AddStretchSpacer();
    sizer->Add(mAddDefineButton);

    return sizer;
}

wxSizer *CustomOptionsDlg::info(wxWindow *parent) {
    auto *infoSizer{new PCUI::StaticBox(
        wxVERTICAL,
        parent,
        _("Links For Additional ProffieOS Defines")
    )};

    auto *text{new wxStaticText(
        infoSizer->GetStaticBox(),
        wxID_ANY,
        _("(ProffieConfig already handles some of these)\n")
    )};
    auto *optDefines{new wxHyperlinkCtrl(
        infoSizer->GetStaticBox(),
        wxID_ANY,
        _("Optional Defines"),
        "https://pod.hubbe.net/config/the-config_top-section.html#optional-defines"
    )};
    auto *clashSuppress{new wxHyperlinkCtrl(
        infoSizer->GetStaticBox(),
        wxID_ANY,
        _("History of Clash Detection"),
        "https://pod.hubbe.net/explainers/history-of-clash.html"
    )};
    infoSizer->Add(
        text,
        wxSizerFlags(0).Border(wxLEFT | wxTOP | wxRIGHT, 10)
    );
    infoSizer->Add(
        optDefines,
        wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10)
    );
    infoSizer->Add(
        clashSuppress,
        wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 10)
    );

    return infoSizer->underlyingSizer();
}

void CustomOptionsDlg::createOptionArea() {
    mOptionArea = new wxScrolledWindow(this, wxID_ANY);
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    mOptionArea->SetScrollRate(10, 10);
    mOptionArea->SetSizerAndFit(sizer);
}

void CustomOptionsDlg::handleNotification(uint32) {
    mOptionArea->GetSizer()->Clear(true);

    auto& config{mParent->getOpenConfig()};
    const auto& customOptions{config.settings.customOptions()};
    if (customOptions.empty()) {
        mOptionArea->GetSizer()->AddStretchSpacer();
        mOptionArea->GetSizer()->Add(
            new wxStaticText(
                    mOptionArea,
                    wxID_ANY,
                    _("Once you add custom options they'll show up here."),
                    wxDefaultPosition,
                    wxDefaultSize,
                    wxALIGN_CENTER_HORIZONTAL
            ),
            wxSizerFlags(0).Expand()
        );
        mOptionArea->GetSizer()->AddStretchSpacer();
    } else {
        for (const auto& constOption: customOptions) {
            auto& option{const_cast<Config::Settings::CustomOption&>(*constOption)};
            mOptionArea->GetSizer()->Add(
                new CDefine(mOptionArea, config, option),
                wxSizerFlags(0).Expand().Border(wxBOTTOM, 5)
            );
        }
    }

    Layout();
}


CustomOptionsDlg::CDefine::CDefine(
    wxScrolledWindow *parent,
    Config::Config& config,
    Config::Settings::CustomOption& option
) : wxPanel(parent, wxID_ANY) {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *defText {new wxStaticText(this, wxID_ANY, "#define")};
    auto *name{new PCUI::Text(this, option.define)};
    auto *value{new PCUI::Text(this, option.value)};
    value->SetMinSize(wxSize{50, -1});
    auto *remove = new wxButton(this, wxID_ANY, _("Remove"));
    remove->Bind(wxEVT_BUTTON, [&config, &option](wxCommandEvent&) {
        config.settings.removeCustomOption(option);
    });

    sizer->Add(defText, wxSizerFlags(0).Center().Border(wxRIGHT, 5));
    sizer->Add(name, wxSizerFlags(5).Border(wxRIGHT, 5));
    sizer->Add(value, wxSizerFlags(3).Border(wxRIGHT, 5));
    sizer->Add(remove, wxSizerFlags(0).Border(wxRIGHT, 10));

    SetSizerAndFit(sizer);
}
