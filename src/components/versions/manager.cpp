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

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>

#include "app/app.h"
#include "ui/controls/text.h"
#include "ui/controls/version.h"

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
    ID_PropRemoveVersion,
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
    Show();
}

void Versions::Manager::bindEvents() {
}

void Versions::Manager::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *notebook{new wxNotebook(this, wxID_ANY)};

    auto *propPage{new wxPanel(notebook)};
    auto *propSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *propListSizer{new wxBoxSizer(wxVERTICAL)};
    auto *propList{new wxListBox(propPage, ID_PropList)};
    auto *propModSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *propAdd{new wxButton(
        propPage,
        ID_PropAdd,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *propRemove{new wxButton(
        propPage,
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
    auto *propName{new PCUI::Text(
        propPage,
        mPropNameProxy,
        0,
        false,
        _("Prop Name")
    )};
    auto *propInfo{new wxStaticText(propPage, ID_PropInfo, {})};
    mPropVersionsSizer = new wxWrapSizer;
    auto *propVersion{new PCUI::Version(propPage, mPropVersionProxy)};
    auto *propDeleteVersion{
        new wxButton(propPage, ID_PropRemoveVersion, _("Remove"))
    };
    propEditSizer->Add(propName, 0, wxEXPAND);
    propEditSizer->AddSpacer(5);
    propEditSizer->Add(propInfo, 0, wxEXPAND);
    propEditSizer->AddSpacer(10);
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
    propPage->SetSizerAndFit(propSizer);

    auto *osPage{new wxPanel(notebook)};

    notebook->AddPage(propPage, _("Prop Files"), true);
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

