#include "addconfig.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/addconfig.cpp
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

#include <wx/sysopt.h>

#include "config/config.h"
#include "ui/notifier.h"
#include "utils/image.h"

AddConfig::AddConfig(MainMenu *parent) : 
    wxDialog(
        parent,
        wxID_ANY,
        _("Add New Config"),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE
    ) {
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
        mImportButton->SetValue(true);
        mCreateButton->SetValue(false);
        update(); 
    }, eID_Import_Existing);
    Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { 
        mCreateButton->SetValue(true);
        mImportButton->SetValue(false);
        importPath_ = filepath{};
        update(); 
    }, eID_Create_New);

    configName_.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;
        update();
    });
    importPath_.setUpdateHandler([this](uint32 id) {
        if (id != pcui::FilePickerData::eID_Path) return;
        configName_ = static_cast<filepath>(importPath_).stem().string();
    });
}

void AddConfig::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    sizer->SetMinSize(wxSize(400, -1));

    auto *addModeSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *createNewToggle{
        new wxToggleButton(this, eID_Create_New, _("Create New Config"))
    };
    createNewToggle->SetValue(true);
    createNewToggle->SetBitmap(Image::loadPNG("new", wxSize{32, -1}));
    auto *importExistingToggle{new wxToggleButton(
        this, eID_Import_Existing, _("Import Existing Config")
    )};
    importExistingToggle->SetBitmap(Image::loadPNG("import", wxSize{32, -1}));
    addModeSizer->Add(
        createNewToggle,
        wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10)
    );
    addModeSizer->Add(
        importExistingToggle,
        wxSizerFlags(0).Border(wxRIGHT | wxTOP | wxBOTTOM, 10)
    );

    auto *importPicker{new pcui::FilePicker(
        this,
        importPath_,
        wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
        _("Configuration to Import"),
        _("ProffieOS Configuration") + " (*.h)|*.h",
        _("Choose Configuration File to Import")
    )};
    auto *nameEntry{new pcui::Text(
        this,
        configName_,
        0,
        false,
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
            if (static_cast<string>(configName_) == config) return true;
        }
        return false;
    }()};

    const auto& configNameText{static_cast<string>(configName_)};
    bool configNameEmpty{configNameText.empty()};
    bool configNameInvalidCharacters{
        configNameText.find_first_of(".\\,/!#$%^&*|?<>\"'") != string::npos
    };
    bool validConfigName{
        not configNameEmpty and
        not duplicateConfigName and
        not configNameInvalidCharacters
    };
    bool importingConfig{mImportButton->GetValue()};
    bool originFileSelected{not static_cast<filepath>(importPath_).empty()};

    mDuplicateWarning->Show(duplicateConfigName);
    mInvalidNameWarning->Show(not validConfigName);
    mFileSelectionWarning->Show(importingConfig and not originFileSelected);

    FindWindow(wxID_OK)->Enable(
        validConfigName and (originFileSelected or not importingConfig)
    );

    importPath_.show(importingConfig);
    pcui::NotifyReceiver::sync();

    GetSizer()->Layout();
    GetSizer()->Fit(this);
}

