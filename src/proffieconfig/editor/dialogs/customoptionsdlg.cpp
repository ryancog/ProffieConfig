#include "customoptionsdlg.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>

CustomOptionsDlg::CustomOptionsDlg(EditorWindow *_parent) : 
    wxDialog(_parent, wxID_ANY, _("Custom Options") + " - " + wxString{_parent->getOpenConfig()}, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    createUI();

    updateOptions();
    bindEvents();
}

void CustomOptionsDlg::addDefine(const wxString& name, const wxString& value) {
    auto *newDefine{new CDefine(mOptionArea)};
    newDefine->name->entry()->SetValue(name);
    newDefine->value->entry()->SetValue(value);
    mCustomDefines.push_back(newDefine);

    newDefine->Bind(wxEVT_BUTTON, [newDefine, this](wxCommandEvent&) {
        mOptionArea->GetSizer()->Detach(newDefine);
        mCustomDefines.erase(std::find(mCustomDefines.begin(), mCustomDefines.end(), newDefine));
        newDefine->Destroy();
        updateOptions();
    }, CDefine::ID_Remove);

    updateOptions();
}

vector<std::pair<wxString, wxString>> CustomOptionsDlg::getCustomDefines() {
    vector<std::pair<wxString, wxString>> outputDefines;
    outputDefines.reserve(mCustomDefines.size());
    for (const auto& define : mCustomDefines) {
        outputDefines.emplace_back(define->name->entry()->GetValue().ToStdString(), define->value->entry()->GetValue().ToStdString());
    }
    return outputDefines;
}

void CustomOptionsDlg::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        updateOptions(true);
        if (event.CanVeto()) {
            Hide();
            event.Veto();
        } else event.Skip();
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        addDefine("");
    }, ID_AddDefine);
}

void CustomOptionsDlg::createUI() {
  auto *sizer{new wxBoxSizer(wxVERTICAL)};

  createOptionArea();

  sizer->Add(header(), wxSizerFlags(0).Expand().Border(wxALL, 10));
  sizer->Add(mOptionArea, wxSizerFlags(1).Expand().Border(wxALL, 10));
  sizer->Add(info(this), wxSizerFlags(0).Expand().Border(wxALL, 10));
  sizer->SetMinSize(450, 500);

  SetSizerAndFit(sizer);
}

wxBoxSizer *CustomOptionsDlg::header() {
  auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

  auto *text{new wxStaticText(this, wxID_ANY, _("Defines are in the format:\n#define [NAME] [VALUE]"))};
  mAddDefineButton = new wxButton(this, ID_AddDefine, _("Add Custom Define"));
  sizer->Add(text, wxSizerFlags(0).Center());
  sizer->AddSpacer(50);
  sizer->AddStretchSpacer();
  sizer->Add(mAddDefineButton);

  return sizer;
}

wxStaticBoxSizer *CustomOptionsDlg::info(wxWindow* parent) {
  auto *infoSizer{new wxStaticBoxSizer(wxVERTICAL, parent, _("Links For Additional ProffieOS Defines"))};

  auto *text{new wxStaticText(infoSizer->GetStaticBox(), wxID_ANY, _("(ProffieConfig already handles some of these)\n"))};
  auto *optDefines{new wxHyperlinkCtrl(infoSizer->GetStaticBox(), wxID_ANY, _("Optional Defines"), "https://pod.hubbe.net/config/the-config_top-section.html#optional-defines")};
  auto *clashSuppress{new wxHyperlinkCtrl(infoSizer->GetStaticBox(), wxID_ANY, _("History of Clash Detection"), "https://pod.hubbe.net/explainers/history-of-clash.html")};
  infoSizer->Add(text, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxRIGHT, 10));
  infoSizer->Add(optDefines, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10));
  infoSizer->Add(clashSuppress, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 10));

  return infoSizer;
}

void CustomOptionsDlg::createOptionArea() {
  mOptionArea = new wxScrolledWindow(this, wxID_ANY);
  auto *sizer{new wxBoxSizer(wxVERTICAL)};

  mCricketsText = new wxStaticText(mOptionArea, wxID_ANY, _("Once you add custom options they'll show up here."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

  mOptionArea->SetScrollRate(0, 10);
  mOptionArea->SetSizerAndFit(sizer);
}

void CustomOptionsDlg::updateOptions(bool purge) {
    mOptionArea->GetSizer()->Clear();

    if (mCustomDefines.empty()) {
        mCricketsText->Show();
        mOptionArea->GetSizer()->AddStretchSpacer();
        mOptionArea->GetSizer()->Add(mCricketsText, wxSizerFlags(0).Expand());
        mOptionArea->GetSizer()->AddStretchSpacer();
    } else {
        mCricketsText->Hide();
        for (auto it = mCustomDefines.begin(); it != mCustomDefines.end();) {
            if (purge && (*it)->name->entry()->GetValue().empty()) {
                mOptionArea->GetSizer()->Detach(*it);
                (*it)->Destroy();

                it = mCustomDefines.erase(it);
                continue;
            }

            mOptionArea->GetSizer()->Add(*(it++), wxSizerFlags(0).Expand().Border(wxBOTTOM, 5));
        }
    }

    Layout();
}

CustomOptionsDlg::CDefine::CDefine(wxScrolledWindow* _parent) : wxPanel(_parent, wxID_ANY) {
  auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

  defText = new wxStaticText(this, wxID_ANY, "#define");
  name = new PCUI::Text(this, ID_Name);
  value = new PCUI::Text(this, ID_Value);
  value->SetMinSize(wxSize{50, -1});
  remove = new wxButton(this, ID_Remove, _("Remove"));

  sizer->Add(defText, wxSizerFlags(0).Center().Border(wxRIGHT, 5));
  sizer->Add(name, wxSizerFlags(3).Border(wxRIGHT, 5));
  sizer->Add(value, wxSizerFlags(1).Border(wxRIGHT, 5));
  sizer->Add(remove, wxSizerFlags(0).Border(wxRIGHT, 10));

  SetSizerAndFit(sizer);
}
