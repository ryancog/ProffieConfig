// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "customoptionsdlg.h"

#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/statbox.h>

CustomOptionsDlg::CustomOptionsDlg(EditorWindow* _parent) : wxDialog(_parent, wxID_ANY, "Custom Options - " + _parent->getOpenConfig(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
  createUI();

  updateOptions();
  bindEvents();
}

void CustomOptionsDlg::addDefine(const std::string& name, const std::string& value) {
    auto newDefine = new CDefine(optionArea);
    newDefine->name->entry()->SetValue(name);
    newDefine->value->entry()->SetValue(value);
    customDefines.push_back(newDefine);

    newDefine->Bind(wxEVT_BUTTON, [newDefine, this](wxCommandEvent) {
        optionArea->GetSizer()->Detach(newDefine);
        customDefines.erase(std::find(customDefines.begin(), customDefines.end(), newDefine));
        newDefine->Destroy();
        updateOptions();
    }, CDefine::ID_Remove);

    updateOptions();
}

std::vector<std::pair<std::string, std::string>> CustomOptionsDlg::getCustomDefines() {
  std::vector<std::pair<std::string, std::string>> outputDefines;
  for (const auto& define : customDefines) {
    outputDefines.push_back({ define->name->entry()->GetValue().ToStdString(), define->value->entry()->GetValue().ToStdString() });
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
  auto sizer = new wxBoxSizer(wxVERTICAL);

  createOptionArea();

  sizer->Add(header(), wxSizerFlags(0).Expand().Border(wxALL, 10));
  sizer->Add(optionArea, wxSizerFlags(1).Expand().Border(wxALL, 10));
  sizer->Add(info(this), wxSizerFlags(0).Expand().Border(wxALL, 10));
  sizer->SetMinSize(450, 500);

  SetSizerAndFit(sizer);
}

wxBoxSizer* CustomOptionsDlg::header() {
  auto sizer = new wxBoxSizer(wxHORIZONTAL);

  auto text = new wxStaticText(this, wxID_ANY, "Defines are in the format:\n#define [NAME] [VALUE]");
  addDefineButton = new wxButton(this, ID_AddDefine, "Add Custom Define");
  sizer->Add(text, wxSizerFlags(0).Center());
  sizer->AddStretchSpacer();
  sizer->Add(addDefineButton);

  return sizer;
}

wxStaticBoxSizer* CustomOptionsDlg::info(wxWindow* parent) {
  auto infoSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Links For Additional ProffieOS Defines");

  auto text = new wxStaticText(infoSizer->GetStaticBox(), wxID_ANY, "(ProffieConfig already handles some of these)\n");
  auto optDefines = new wxHyperlinkCtrl(infoSizer->GetStaticBox(), wxID_ANY, "Optional Defines", "https://pod.hubbe.net/config/the-config_top-section.html#optional-defines");
  auto clashSuppress = new wxHyperlinkCtrl(infoSizer->GetStaticBox(), wxID_ANY, "History of Clash Detection", "https://pod.hubbe.net/explainers/history-of-clash.html");
  infoSizer->Add(text, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxRIGHT, 10));
  infoSizer->Add(optDefines, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10));
  infoSizer->Add(clashSuppress, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 10));

  return infoSizer;
}

void CustomOptionsDlg::createOptionArea() {
  optionArea = new wxScrolledWindow(this, wxID_ANY);
  auto sizer = new wxBoxSizer(wxVERTICAL);

  cricketsText = new wxStaticText(optionArea, wxID_ANY, "Once you add custom options they'll show up here.", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

  optionArea->SetScrollRate(0, 10);
  optionArea->SetSizerAndFit(sizer);
}

void CustomOptionsDlg::updateOptions(bool purge) {
    optionArea->GetSizer()->Clear();

    if (customDefines.empty()) {
        cricketsText->Show();
        optionArea->GetSizer()->AddStretchSpacer();
        optionArea->GetSizer()->Add(cricketsText, wxSizerFlags(0).Expand());
        optionArea->GetSizer()->AddStretchSpacer();
    } else {
        cricketsText->Hide();
        for (auto it = customDefines.begin(); it != customDefines.end();) {
            if (purge && (*it)->name->entry()->GetValue().empty()) {
                optionArea->GetSizer()->Detach(*it);
                (*it)->Destroy();

                it = customDefines.erase(it);
                continue;
            }

            optionArea->GetSizer()->Add(*(it++), wxSizerFlags(0).Expand().Border(wxBOTTOM, 5));
        }
    }

    Layout();
}

CustomOptionsDlg::CDefine::CDefine(wxScrolledWindow* _parent) : wxPanel(_parent, wxID_ANY) {
  auto sizer = new wxBoxSizer(wxHORIZONTAL);

  defText = new wxStaticText(this, wxID_ANY, "#define");
  name = new pcTextCtrl(this, ID_Name);
  value = new pcTextCtrl(this, ID_Value, wxEmptyString, wxDefaultPosition, wxSize(50, -1));
  remove = new wxButton(this, ID_Remove, "Remove");

  sizer->Add(defText, wxSizerFlags(0).Center().Border(wxRIGHT, 5));
  sizer->Add(name, wxSizerFlags(3).Border(wxRIGHT, 5));
  sizer->Add(value, wxSizerFlags(1).Border(wxRIGHT, 5));
  sizer->Add(remove, wxSizerFlags(0).Border(wxRIGHT, 10));

  SetSizerAndFit(sizer);
}
