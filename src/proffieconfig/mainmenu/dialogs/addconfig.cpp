#include "addconfig.h"
#include "config/config.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/sysopt.h>

AddConfig::AddConfig(MainMenu *parent) : 
    wxDialog(parent, wxID_ANY, _("Add New Config"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) {
#   ifdef __WXOSX__
    wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, true);
#   endif

    createUI();
    bindEvents();

    FindWindow(wxID_OK)->Disable();
}

void AddConfig::bindEvents() {
    // We make sure to set itself to true that way it can't be deselected
    Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { 
        static_cast<wxToggleButton *>(FindWindow(ID_ImportExisting))->SetValue(true);
        static_cast<wxToggleButton *>(FindWindow(ID_CreateNew))->SetValue(false);
        update(); 
    }, ID_ImportExisting);
    Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { 
        static_cast<wxToggleButton *>(FindWindow(ID_CreateNew))->SetValue(true);
        static_cast<wxToggleButton *>(FindWindow(ID_ImportExisting))->SetValue(false);
        importPath = filepath{};
        update(); 
    }, ID_CreateNew);
    configName.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::TextData::ID_VALUE) return;
        update();
    });
    importPath.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::FilePickerData::ID_PATH) return;
        configName = static_cast<filepath>(importPath).stem();
    });
}

void AddConfig::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    sizer->SetMinSize(wxSize(400, -1));

    auto *addModeSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *createNewToggle{
        new wxToggleButton(this, ID_CreateNew, _("Create New Config"))
    };
    createNewToggle->SetValue(true);
    auto *importExistingToggle{
        new wxToggleButton(this, ID_ImportExisting, _("Import Existing Config"))
    };
    addModeSizer->Add(
        createNewToggle,
        wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10)
    );
    addModeSizer->Add(
        importExistingToggle,
        wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10)
    );

    auto *importPicker{new PCUI::FilePicker(
        this,
        importPath,
        wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
        _("Configuration to Import"),
        "ProffieOS Configuration (*.h)|*.h",
        _("Choose Configuration File to Import")
    )};
    auto *nameEntry{new PCUI::Text(
        this,
        configName,
        0,
        _("Configuration Name")
    )};

    mInvalidNameWarning = new wxStaticText(
        this,
        wxID_ANY,
        _("Please enter a valid name")
    );
    mDuplicateWarning = new wxStaticText(
        this,
        wxID_ANY,
        _("Configuration with same name already exists")
    );
    mFileSelectionWarning = new wxStaticText(
        this,
        wxID_ANY,
        _("Please choose a configuration file to import")
    );

    sizer->Add(addModeSizer, wxSizerFlags(0).Center());
    sizer->Add(
        importPicker,
        wxSizerFlags(0).Expand().Border(wxALL, 10)
    );
    sizer->Add(
        nameEntry,
        wxSizerFlags(0)
            .Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10)
    );
    sizer->Add(
        mInvalidNameWarning,
        wxSizerFlags(0).Right().Border(wxRIGHT, 10)
    );
    sizer->Add(
        mDuplicateWarning,
        wxSizerFlags(0).Right().Border(wxRIGHT, 10)
    );
    sizer->Add(
        mFileSelectionWarning,
        wxSizerFlags(0).Right().Border(wxRIGHT, 10)
    );
    sizer->AddStretchSpacer(1);
    sizer->Add(
        CreateStdDialogButtonSizer(wxOK | wxCANCEL),
        wxSizerFlags(0).Border(wxALL, 10).Expand()
    );

    SetSizerAndFit(sizer);
    update();
}

void AddConfig::update() {
    auto duplicateConfigName{[&]() { 
        for (const auto& config : Config::fetchListFromDisk()) {
            if (static_cast<string>(configName) == config) return true;
        }
        return false;
    }()};

    const auto& configNameText{static_cast<string>(configName)};
    bool configNameEmpty{configNameText.empty()};
    bool configNameInvalidCharacters{
        configNameText.find_first_of(".\\,/!#$%^&*|?<>\"'") != string::npos
    };
    bool validConfigName{not configNameEmpty and not duplicateConfigName and not configNameInvalidCharacters};
    bool importingConfig{static_cast<wxToggleButton *>(FindWindow(ID_ImportExisting))->GetValue()};
    bool originFileSelected{not static_cast<filepath>(importPath).empty()};

    mDuplicateWarning->Show(duplicateConfigName);
    mInvalidNameWarning->Show(not validConfigName);
    mFileSelectionWarning->Show(importingConfig and not originFileSelected);

    FindWindow(wxID_OK)->Enable(validConfigName and (originFileSelected or not importingConfig));

    importPath.show(importingConfig);

    GetSizer()->Layout();
    GetSizer()->Fit(this);
}
