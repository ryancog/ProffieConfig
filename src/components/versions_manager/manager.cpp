#include "manager.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/versions_manager/manager.cpp
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
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>

#include "app/app.h"
#include "config/config.h"
#include "log/context.h"
#include "log/severity.h"
#include "ui/controls/version.h"
#include "ui/frame.h"
#include "ui/message.h"
#include "utils/defer.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "versions/versions.h"

namespace VersionsManager {

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

    ID_OSList,
    ID_OSAdd,
    ID_OSRemove,
    ID_OSText,
    
    ID_PropVersionListAdd,
    ID_PropVersionListBegin,
};

void bindEvents();

void reloadFromDisk();

void updatePropList();
void updatePropSelection();
void updatePropVersionList();
void updatePropVersionSelection();

void updateOSList();
void updateOSSelection();

void updateSizeAndLayout();

void createUI();
void createMenuBar();

int32 getVersionSelection();

PCUI::Frame *manager{nullptr};

PCUI::VersionDataProxy mPropVersionProxy;
wxPanel *mPropPage;
wxWrapSizer *mPropVersionsSizer;

wxPanel *mOSPage;

} // namespace VersionsManager

void VersionsManager::open(wxWindow *parent, wxWindowID id) {
    if (manager) {
        manager->Raise();
        return;
    }

    manager = new PCUI::Frame(parent, id, _("Versions Manager"));
    manager->Bind(wxEVT_CLOSE_WINDOW, [](wxCloseEvent& evt) {
        manager = nullptr;
        evt.Skip();
    });

    
    createUI();
    createMenuBar();
    bindEvents();

    updatePropList();
    updateOSList();

    manager->Show();
}

void VersionsManager::bindEvents() {
    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        wxDialog dlg(manager, wxID_ANY, _("Add Prop"));
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
            constexpr cstring LOG_TAG{"VersionsManager Add Prop"};
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
            if (not fs::copy_file(file->GetPath().ToStdString(), propDir / Versions::DATA_FILE_STR, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to move prop data file: " + err.message()
                );
                fs::remove_all(propDir, err);
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }
            if (not fs::copy_file(header->GetPath().ToStdString(), propDir / Versions::HEADER_FILE_STR, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to move prop header file: " + err.message()
                );
                fs::remove_all(propDir, err);
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }

            reloadFromDisk();
        }
    }, ID_PropAdd);
    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        auto res{PCUI::showMessage(
            _("This cannot be undone!"), _("Remove Prop"),
            wxYES_NO | wxNO_DEFAULT, manager
        )};
        if (res == wxYES) {
            const auto& versionedProp{
                Versions::getProps()[static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()]
            };
            fs::remove_all(Paths::propDir() / versionedProp->name);
        }

        reloadFromDisk();
    }, ID_PropRemove);
    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const auto& versionedProp{
            Versions::getProps()[static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()]
        };
        versionedProp->addVersion();
        updatePropVersionList();
    }, ID_PropVersionListAdd);
    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const auto& versionedProp{
            Versions::getProps()[static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()]
        };
        const auto selectedVersion{getVersionSelection()};
        if (selectedVersion == -1) return;

        versionedProp->removeVersion(selectedVersion);
        updatePropVersionList();
    }, ID_PropRemoveVersion);

    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        wxDialog dlg(manager, wxID_ANY, _("Add ProffieOS"));
        auto *sizer{new wxBoxSizer(wxVERTICAL)};
        PCUI::VersionData versionData;
        auto *version{new PCUI::Version(&dlg, versionData, _("Version"))};
        auto *duplicateMessage{new wxStaticText(
            &dlg,
            wxID_ANY,
            _("Duplicate OS Version")
        )};
        auto *folder{new wxDirPickerCtrl(
            &dlg,
            wxID_ANY,
            wxEmptyString,
            _("Select ProffieOS Folder"),
            wxDefaultPosition,
            wxDefaultSize,
            wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST
        )};
        auto flags{wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)};
        sizer->AddSpacer(10);
        sizer->Add(version, flags);
        sizer->Add(duplicateMessage, flags);
        sizer->AddSpacer(5);
        sizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Folder")), flags);
        sizer->Add(folder, flags);
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

        const auto updateState{[&dlg, &versionData, duplicateMessage, folder]() {
            bool duplicate{false};
            const auto versionStr{static_cast<string>(versionData)};
            for (const auto& entry : fs::directory_iterator{Paths::propDir()}) {
                if (entry.path().filename() == versionStr) {
                    duplicate = true;
                    break;
                }
            }

            duplicateMessage->Show(duplicate);
            const auto folderPath{folder->GetPath().ToStdString()};

            std::error_code err;
            dlg.FindWindow(wxID_OK)->Enable(
                not duplicate and
                not versionStr.empty() and
                fs::exists(folderPath, err)
            );

            dlg.Fit();
        }};

        versionData.setUpdateHandler([updateState](uint32 id) {
            if (id != PCUI::VersionData::ID_VALUE) return;
            updateState();
        });
        folder->Bind(wxEVT_DIRPICKER_CHANGED, [updateState](wxCommandEvent&) {
            updateState();
        });
        
        updateState();
        if (dlg.ShowModal() == wxID_OK) {
            const auto errorTitleStr{_("Failed to Add ProffieOS")};
            const auto errorStr{_("OS FS Failure")};
            constexpr cstring LOG_TAG{"VersionsManager Add Prop"};
            std::error_code err;
            const auto osDir{Paths::osDir() / static_cast<string>(versionData)};
            if (not fs::create_directory(osDir, err)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to create directory " + osDir.string() + ", " + err.message()
                );
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }

            const auto copyOpts{fs::copy_options::recursive};
            fs::copy(folder->GetPath().ToStdString(), osDir / "ProffieOS", copyOpts, err);
            if (err) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::WARN, LOG_TAG,
                    "Failed to move prop data file: " + err.message()
                );
                fs::remove_all(osDir, err);
                PCUI::showMessage(errorStr, errorTitleStr);
                return;
            }

            reloadFromDisk();
        }
    }, ID_OSAdd);
    manager->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        auto res{PCUI::showMessage(
            _("This cannot be undone!"), _("Remove OS"),
            wxYES_NO | wxNO_DEFAULT, manager
        )};
        if (res == wxYES) {
            const auto& versionedOS{
                Versions::getOSVersions()[static_cast<wxListBox *>(manager->FindWindow(ID_OSList))->GetSelection()]
            };
            fs::remove_all(Paths::osDir() / static_cast<string>(versionedOS.verNum));
        }

        reloadFromDisk();
    }, ID_OSRemove);
    
    manager->Bind(wxEVT_LISTBOX, [](wxCommandEvent&) {
        updatePropSelection();
    }, ID_PropList);
    manager->Bind(wxEVT_LISTBOX, [](wxCommandEvent&) {
        updateOSSelection();
    }, ID_OSList);

    manager->Bind(wxEVT_MENU, [](wxCommandEvent&) {
        reloadFromDisk();
    }, ID_Refresh);
    manager->Bind(wxEVT_MENU, [](wxCommandEvent&) {
        wxLaunchDefaultApplication(Paths::versionDir().string());
    }, ID_ShowVersions);
    manager->Bind(wxEVT_MENU, [](wxCommandEvent&) {
        constexpr auto FULL_RESET{wxID_YES};
        constexpr auto RESTORE_DEFAULTS{wxID_NO};
        auto res{PCUI::showMessage(
            _("These actions cannot be undone!\n"
            "\n"
            "\"Restore Defaults\" will replace/restore default versions, reverting them but preserving any custom versions.\n"
            "\n"
            "\"Full Reset\" will replace/restore default versions and additionally remove any/all custom versions."),
            _("Restore Default Versions"),
            wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT,
            manager
        )};

        if (res == wxCANCEL) return;

        bool confirm{false};
        if (res == RESTORE_DEFAULTS) {
            confirm = wxYES == PCUI::showMessage(
                _("Are you sure?"),
                _("Restore Default Versions"),
                wxYES_NO | wxNO_DEFAULT
            );
        }
        if (res == FULL_RESET) {
            confirm = wxYES == PCUI::showMessage(
                _("Are you sure?"),
                _("Full Versions Reset"),
                wxYES_NO | wxNO_DEFAULT
            );
        }

        if (not confirm) return;

        // Alr, now do it.
    }, ID_Reset);
}

void VersionsManager::reloadFromDisk() {
    Versions::loadLocal();

    for (const auto& config : Config::getOpen()) {
        config->refreshOSVersions();
        config->refreshPropVersions();
    }

    updatePropList();
    updateOSList();
}

void VersionsManager::updatePropList() {
    auto *propList{static_cast<wxListBox *>(manager->FindWindow(ID_PropList))};
    const auto oldSelection{propList->GetSelection()};

    propList->Clear();
    for (const auto& versionedProp : Versions::getProps()) {
        propList->Append(static_cast<string>(versionedProp->name));
    }

    if (not propList->IsEmpty()) {
        propList->SetSelection(
            oldSelection == -1 ?
            oldSelection :
            std::clamp<int32>(oldSelection, 0,  propList->GetCount() - 1)
        );
    }

    updatePropSelection();
}

void VersionsManager::updatePropSelection() {
    const auto selection{static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()};
    manager->FindWindow(ID_PropRemove)->Enable(selection != -1);
    manager->FindWindow(ID_PropVersionsText)->Enable(selection != -1);
    auto *infoText{static_cast<wxStaticText *>(manager->FindWindow(ID_PropInfo))};

    if (selection != -1) {
        const auto& versionedProp{Versions::getProps()[selection]};

        infoText->SetLabelText(
            "Name: " + versionedProp->name + "\n"
            "Display Name: " + versionedProp->prop->name + "\n"
            "Header File: " + versionedProp->prop->filename
        );
    }

    infoText->Show(selection != -1);
    updatePropVersionList();
}

void VersionsManager::updatePropVersionList() {
    Defer defer{[]() { updatePropVersionSelection(); }};
    const auto oldSelection{getVersionSelection()};

    const auto propSelection{
        static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()
    };

    mPropVersionsSizer->Clear(true);
    if (propSelection == -1) return;

    const auto& versionedProp{Versions::getProps()[propSelection]};
    int32 id{ID_PropVersionListBegin};
    for (const auto& version : versionedProp->supportedVersions()) {
        const auto toggleLabel{static_cast<string>(static_cast<Utils::Version>(*version))};
        auto *toggle{new wxToggleButton(
            mPropPage,
            id,
            toggleLabel
        )};
        toggle->Bind(wxEVT_TOGGLEBUTTON, [toggle](wxCommandEvent&) {
            if (toggle->GetValue()) {
                for (auto *child : mPropVersionsSizer->GetChildren()) {
                    auto *childTogglePtr{dynamic_cast<wxToggleButton *>(child->GetWindow())};
                    if (childTogglePtr == toggle) continue;
                    
                    if (childTogglePtr) childTogglePtr->SetValue(false);
                }
            }
            updatePropVersionSelection();
        });
        toggle->SetMinSize({toggle->GetTextExtent(toggleLabel).x + 10, -1});
        if (id - ID_PropVersionListBegin == oldSelection) toggle->SetValue(true);
        mPropVersionsSizer->Add(toggle);
        ++id;
    }
    mPropVersionsSizer->AddSpacer(5);
    mPropVersionsSizer->Add(new wxButton(
        mPropPage,
        ID_PropVersionListAdd,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    ));
}

void VersionsManager::updatePropVersionSelection() {
    const auto versionSelection{getVersionSelection()};

    manager->FindWindow(ID_PropRemoveVersion)->Enable(versionSelection != -1);
    if (versionSelection != -1) {
        const auto& versionedProp{Versions::getProps()[
            static_cast<wxListBox *>(manager->FindWindow(ID_PropList))->GetSelection()
        ]};
        mPropVersionProxy.bind(*versionedProp->supportedVersions()[versionSelection]);
        mPropVersionProxy.data()->setFocus();
    } else mPropVersionProxy.unbind();

    updateSizeAndLayout();
}

void VersionsManager::updateOSList() {
    auto *osList{static_cast<wxListBox *>(manager->FindWindow(ID_OSList))};
    const auto oldSelection{osList->GetSelection()};

    osList->Clear();
    for (const auto& versionedOS : Versions::getOSVersions()) {
        osList->Append(static_cast<string>(versionedOS.verNum));
    }

    if (not osList->IsEmpty()) {
        osList->SetSelection(
            oldSelection == -1 ?
            oldSelection :
            std::clamp<int32>(oldSelection, 0,  osList->GetCount() - 1)
        );
    }

    updateOSSelection();
}

void VersionsManager::updateOSSelection() {
    const auto selection{static_cast<wxListBox *>(manager->FindWindow(ID_OSList))->GetSelection()};
    manager->FindWindow(ID_OSRemove)->Enable(selection != -1);
    auto *infoText{static_cast<wxStaticText *>(manager->FindWindow(ID_OSText))};

    if (selection != -1) {
        const auto& versionedOS{Versions::getOSVersions()[selection]};

        const auto defaultStr{_("Default")};
        const auto coreVersion{versionedOS.coreVersion.err ? defaultStr : static_cast<string>(versionedOS.coreVersion)};
        const auto coreURL{versionedOS.coreURL.empty() ? defaultStr : versionedOS.coreURL};
        const auto coreBoardV1{versionedOS.coreBoardV1.empty() ? defaultStr : versionedOS.coreBoardV1};
        const auto coreBoardV2{versionedOS.coreBoardV2.empty() ? defaultStr : versionedOS.coreBoardV2};
        const auto coreBoardV3{versionedOS.coreBoardV3.empty() ? defaultStr : versionedOS.coreBoardV3};

        infoText->SetLabelText(
            "Version: " + static_cast<string>(versionedOS.verNum) + "\n"
            "Core Version: " + coreVersion + "\n"
            "Core URL: " + coreURL + "\n"
            "Core Board V1: " + coreBoardV1 + "\n"
            "Core Board V2: " + coreBoardV2 + "\n"
            "Core Board V3: " + coreBoardV3
        );
    }

    infoText->Show(selection != -1);

    updateSizeAndLayout();
}

void VersionsManager::updateSizeAndLayout() {
    mPropPage->Layout();
    mPropPage->SetMinSize(mPropPage->GetBestSize());
    mOSPage->Layout();
    mOSPage->SetMinSize(mOSPage->GetBestSize());
    manager->Layout();
    manager->SetMaxSize({-1, -1});
    manager->SetMinSize(manager->GetBestSize());
    manager->Fit();
    manager->SetMaxSize({manager->GetSize().x, -1});
}

void VersionsManager::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *notebook{new wxNotebook(manager, wxID_ANY)};

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
    propVersion->Bind(wxEVT_TEXT, [](wxCommandEvent&) {
        updatePropVersionList();
    });
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
    propSizer->Add(propEditSizer, 0, wxEXPAND);
    propSizer->AddSpacer(10);
    mPropPage->SetSizerAndFit(propSizer);

    mOSPage = new wxPanel(notebook);
    auto *osSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *osListSizer{new wxBoxSizer(wxVERTICAL)};
    auto *osList{new wxListBox(mOSPage, ID_OSList)};
    auto *osModSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *osAdd{new wxButton(
        mOSPage,
        ID_OSAdd,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *osRemove{new wxButton(
        mOSPage,
        ID_OSRemove,
        "-",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    osModSizer->Add(osAdd, 1);
    osModSizer->AddSpacer(5);
    osModSizer->Add(osRemove, 1);
    osListSizer->Add(osList, 1);
    osListSizer->AddSpacer(5);
    osListSizer->Add(osModSizer, 0, wxEXPAND);
    osListSizer->AddSpacer(10);

    auto *osInfoSizer{new wxBoxSizer(wxVERTICAL)};
    auto *osInfo{new wxStaticText(mOSPage, ID_OSText, wxEmptyString)};
    osInfoSizer->Add(osInfo, 1, wxEXPAND);

    osSizer->AddSpacer(10);
    osSizer->Add(osListSizer, 0, wxEXPAND);
    osSizer->AddSpacer(10);
    osSizer->Add(osInfoSizer, 0, wxEXPAND);
    osSizer->AddSpacer(10);
    mOSPage->SetSizerAndFit(osSizer);

    notebook->AddPage(mPropPage, _("Prop Files"), true);
    notebook->AddPage(mOSPage, _("ProffieOS"));

    sizer->Add(notebook, 1, wxEXPAND | wxALL, 10);
    manager->SetSizerAndFit(sizer);
    manager->SetMaxSize({manager->GetSize().x, -1});
}

void VersionsManager::createMenuBar() {
    auto *menuBar{new wxMenuBar};
    auto *file{new wxMenu};
    file->Append(ID_Refresh, _("Reload From Disk") + "\tCtrl+Alt+R");
    file->Append(ID_ShowVersions, _("Show Versions Folder") + "\tCtrl+L");
    file->AppendSeparator();
    file->Append(ID_Reset, _("Update From Server..."));

    menuBar->Append(file, _("&File"));
    App::appendDefaultMenuItems(menuBar);

    manager->SetMenuBar(menuBar);
}

int32 VersionsManager::getVersionSelection() {
    int32 ret{-1};
    for (const auto *selectionButton : mPropVersionsSizer->GetChildren()) {
        auto *toggleButton{
            dynamic_cast<const wxToggleButton *>(selectionButton->GetWindow())
        };
        if (toggleButton) {
            if (toggleButton->GetValue()) ret = toggleButton->GetId() - ID_PropVersionListBegin;
        }
    }

    return ret;
}

