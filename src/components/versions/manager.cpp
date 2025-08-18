#include "manager.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/versions/manager.cpp
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

#include <filesystem>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>

#include "app/app.h"
#include "log/context.h"
#include "log/severity.h"
#include "ui/controls/version.h"
#include "ui/message.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "versions/versions.h"
#include "wx/gdicmn.h"

namespace Versions {

Manager *Manager::smInstance;

enum {
    ID_Refresh = 2,
    ID_Reset,
    ID_ShowVersions,

    ID_PropList,
    ID_PropAdd,
    ID_PropRemove,
    ID_PropInfo,
    ID_PropVersionsText,
    ID_PropRemoveVersion,
    
    ID_PropVersionListAdd,
    ID_PropVersionListBegin,
};

} // namespace Versions

void Versions::Manager::open(wxWindow *parent, wxWindowID id) {
    if (smInstance) smInstance->Raise();
    else smInstance = new Manager(parent, id);
}

bool Versions::Manager::Destroy() {
    smInstance = nullptr;
    return PCUI::Frame::Destroy();
}

Versions::Manager::Manager(wxWindow *parent, wxWindowID id) :
    PCUI::Frame(parent, id, _("Versions Manager")) {
    
    createUI();
    createMenuBar();
    bindEvents();

    updatePropList();
    updatePropVersionList();

    Show();
}

void Versions::Manager::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        wxDialog dlg(this, wxID_ANY, _("Add Prop"));
        auto *sizer{new wxBoxSizer(wxVERTICAL)};
        auto *name{new wxTextCtrl(&dlg, wxID_ANY)};
        auto *duplicateMessage{new wxStaticText(
            &dlg,
            wxID_ANY,
            _("Duplicate Prop Name")
        )};
        auto *file{new wxFilePickerCtrl(
            &dlg,
            wxID_ANY,
            wxEmptyString,
            _("Select Prop Data File"),
            _("PConf File") + " (*.pconf)|*.pconf",
            wxDefaultPosition,
            wxDefaultSize,
            wxFLP_USE_TEXTCTRL | wxFLP_OPEN | wxFLP_FILE_MUST_EXIST
        )};
        auto *header{new wxFilePickerCtrl(
            &dlg,
            wxID_ANY,
            wxEmptyString,
            _("Select Prop Header File"),
            _("C Header") + " (*.h)|*.h",
            wxDefaultPosition,
            wxDefaultSize,
            wxFLP_USE_TEXTCTRL | wxFLP_OPEN | wxFLP_FILE_MUST_EXIST
        )};
        auto flags{wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)};
        sizer->AddSpacer(10);
        sizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Name")), flags);
        sizer->Add(name, flags);
        sizer->Add(duplicateMessage, flags);
        sizer->AddSpacer(5);
        sizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Data File")), flags);
        sizer->Add(file, flags);
        sizer->AddSpacer(5);
        sizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Header File")), flags);
        sizer->Add(header, flags);
#       ifdef __WXOSX__
        flags = wxSizerFlags().Expand();
#       else
        flags = wxSizerFlags().Expand().Border(wxALL, 10);
#       endif
        sizer->Add(
            dlg.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
            flags
        );
        dlg.SetSizerAndFit(sizer);
        dlg.SetMinSize({500, -1});

        const auto updateState{[&dlg, name, duplicateMessage, file, header]() {
            bool duplicate{false};
            for (const auto& entry : fs::directory_iterator{Paths::propDir()}) {
                if (entry.path().filename() == name->GetValue().ToStdString()) {
                    duplicate = true;
                    break;
                }
            }

            duplicateMessage->Show(duplicate);
            const auto filePath{file->GetPath().ToStdString()};
            const auto headerPath{header->GetPath().ToStdString()};

            std::error_code err;
            dlg.FindWindow(wxID_OK)->Enable(
                not duplicate and
                not name->IsEmpty() and
                fs::exists(filePath, err) and
                fs::exists(headerPath, err)
            );

            dlg.Fit();
        }};

        name->Bind(wxEVT_TEXT, [name, updateState](wxCommandEvent&) {
            auto value{name->GetValue().ToStdString()};
            const auto insertionPoint{name->GetInsertionPoint()};
            uint32 numTrimmed{};
            Utils::trimUnsafe(value, &numTrimmed, insertionPoint);
            name->ChangeValue(value);
            name->SetInsertionPoint(insertionPoint - numTrimmed);

            updateState();
        });
        file->Bind(wxEVT_FILEPICKER_CHANGED, [updateState](wxCommandEvent&) {
            updateState();
        });
        header->Bind(wxEVT_FILEPICKER_CHANGED, [updateState](wxCommandEvent&) {
            updateState();
        });
        
        updateState();
        if (dlg.ShowModal() == wxID_OK) {
            const auto errorTitleStr{_("Failed to Add Prop")};
            const auto errorStr{_("OS FS Failure")};
            constexpr cstring LOG_TAG{"Versions::Manager Add Prop"};
            std::error_code err;
            const auto propDir{Paths::propDir() / name->GetValue().ToStdString()};
            if (not fs::create_directory(propDir, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to create directory " + propDir.string() + ", " + err.message()
                );
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }
            if (not fs::copy_file(file->GetPath().ToStdString(), propDir / DATA_FILE_STR, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to move prop data file: " + err.message()
                );
                fs::remove_all(propDir, err);
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }
            if (not fs::copy_file(header->GetPath().ToStdString(), propDir / HEADER_FILE_STR, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to move prop header file: " + err.message()
                );
                fs::remove_all(propDir, err);
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }

            loadLocal();
            updatePropList();
        }
    }, ID_PropAdd);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {

    }, ID_PropRemove);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {

    }, ID_PropVersionListAdd);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {

    }, ID_PropRemoveVersion);
    
    Bind(wxEVT_LISTBOX, [this](wxCommandEvent&) { updatePropSelection(); });
}

void Versions::Manager::updatePropList() {
    auto *propList{static_cast<wxListBox *>(FindWindow(ID_PropList))};
    const auto oldSelection{propList->GetSelection()};

    propList->Clear();
    for (const auto& versionedProp : getProps()) {
        propList->Append(static_cast<string>(versionedProp->name));
    }

    if (not propList->IsEmpty()) {
        propList->SetSelection(
            oldSelection == -1 ?
            oldSelection :
            std::clamp<int32>(oldSelection, 0,  - 1)
        );
    }

    updatePropSelection();
}

void Versions::Manager::updatePropSelection() {
    const auto selection{static_cast<wxListBox *>(FindWindow(ID_PropList))->GetSelection()};
    FindWindow(ID_PropRemove)->Enable(selection != -1);
    FindWindow(ID_PropVersionsText)->Enable(selection != -1);
    auto *infoText{static_cast<wxStaticText *>(FindWindow(ID_PropInfo))};

    if (selection != -1) {
        const auto& versionedProp{getProps()[selection]};

        infoText->SetLabelText(
            "Name: " + versionedProp->name + "\n"
            "Display Name: " + versionedProp->prop->name + "\n"
            "Header File: " + versionedProp->prop->filename
        );
    }

    infoText->Show(selection != 1);
    updatePropVersionList();
}

void Versions::Manager::updatePropVersionList() {
    const auto propSelection{
        static_cast<wxListBox *>(FindWindow(ID_PropList))->GetSelection()
    };
    mPropVersionsSizer->Clear();
    if (propSelection == -1) {
        updatePropVersionSelection();
        return;
    }

    const auto& versionedProp{getProps()[propSelection]};
    int32 id{ID_PropVersionListBegin};
    for (const auto& version : versionedProp->supportedVersions) {
        mPropVersionsSizer->Add(new wxToggleButton(
            mPropPage,
            id,
            static_cast<string>(static_cast<Utils::Version>(*version))
        ));
        ++id;
    }
    mPropVersionsSizer->Add(new wxButton(
        mPropPage,
        ID_PropVersionListAdd,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    ));

    Layout();
    Fit();
    SetMinSize(GetSize());

    updatePropVersionSelection();
}

void Versions::Manager::updatePropVersionSelection() {
    const auto versionSelection{[this]() {
        int32 ret{-1};
        for (const auto *selectionButton : mPropVersionsSizer->GetChildren()) {
            auto *toggleButton{dynamic_cast<const wxToggleButton *>(selectionButton)};
            if (toggleButton) {
                if (toggleButton->GetValue()) ret = toggleButton->GetId() - ID_PropVersionListBegin;
            }
        }

        return ret;
    }()};

    FindWindow(ID_PropRemoveVersion)->Enable(versionSelection != -1);
    if (versionSelection != -1) {
        const auto& versionedProp{getProps()[
            static_cast<wxListBox *>(FindWindow(ID_PropList))->GetSelection()
        ]};
        mPropVersionProxy.bind(*versionedProp->supportedVersions[versionSelection]);
    } else mPropVersionProxy.unbind();
}

void Versions::Manager::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *notebook{new wxNotebook(this, wxID_ANY)};

    mPropPage = new wxPanel(notebook);
    auto *propSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *propListSizer{new wxBoxSizer(wxVERTICAL)};
    auto *propList{new wxListBox(mPropPage, ID_PropList)};
    auto *propModSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *propAdd{new wxButton(
        mPropPage,
        ID_PropAdd,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *propRemove{new wxButton(
        mPropPage,
        ID_PropRemove,
        "-",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    propModSizer->Add(propAdd, 1);
    propModSizer->AddSpacer(5);
    propModSizer->Add(propRemove, 1);
    propListSizer->Add(propList, 1);
    propListSizer->AddSpacer(5);
    propListSizer->Add(propModSizer, 0, wxEXPAND);
    propListSizer->AddSpacer(10);

    auto *propEditSizer{new wxBoxSizer(wxVERTICAL)};
    auto *propInfo{new wxStaticText(mPropPage, ID_PropInfo, {})};
    mPropVersionsSizer = new wxWrapSizer;
    auto *propVersion{new PCUI::Version(mPropPage, mPropVersionProxy)};
    auto *propDeleteVersion{
        new wxButton(mPropPage, ID_PropRemoveVersion, _("Remove Version"))
    };
    propEditSizer->Add(propInfo, 0, wxEXPAND);
    propEditSizer->AddSpacer(10);
    propEditSizer->Add(
        new wxStaticText(mPropPage, ID_PropVersionsText, _("Supported OS Versions"))
    );
    propEditSizer->Add(mPropVersionsSizer, 1, wxEXPAND);
    propEditSizer->AddSpacer(5);
    propEditSizer->Add(propVersion, 0, wxEXPAND);
    propEditSizer->AddSpacer(5);
    propEditSizer->Add(propDeleteVersion, 0, wxEXPAND);
    propEditSizer->AddSpacer(10);

    propSizer->AddSpacer(10);
    propSizer->Add(propListSizer, 0, wxEXPAND);
    propSizer->AddSpacer(10);
    propSizer->Add(propEditSizer);
    propSizer->AddSpacer(10);
    mPropPage->SetSizerAndFit(propSizer);

    auto *osPage{new wxPanel(notebook)};

    notebook->AddPage(mPropPage, _("Prop Files"), true);
    notebook->AddPage(osPage, _("ProffieOS"));

    sizer->Add(notebook, 1, wxEXPAND | wxALL, 10);
    SetSizerAndFit(sizer);
    SetMaxSize({GetSize().x, -1});
}

void Versions::Manager::createMenuBar() {
    auto *menuBar{new wxMenuBar};
    auto *file{new wxMenu};
    file->Append(ID_Refresh, _("Reload From Disk") + "\tCtrl+Alt+R");
    file->Append(ID_ShowVersions, _("Show Versions Folder") + "\tCtrl+L");
    file->AppendSeparator();
    file->Append(ID_Reset, _("Update From Server..."));

    menuBar->Append(file, _("&File"));
    App::appendDefaultMenuItems(menuBar);

    SetMenuBar(menuBar);
}

