#include "customoptionsdlg.h"

#include <wx/hyperlink.h>

CustomOptionsDlg::CustomOptionsDlg(EditorWindow* _parent) : wxDialog(_parent, wxID_ANY, "Custom Options - " + _parent->getOpenConfig(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), parent(_parent) {
  createUI();

  updateOptions();
  bindEvents();
}

void CustomOptionsDlg::addDefine(const std::string& name, const std::string& value) {
  auto newDefine = new CDefine(optionArea);
  newDefine->name->entry()->SetValue(name);
  newDefine->value->entry()->SetValue(value);
  customDefines.push_back(newDefine);
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
    if (event.CanVeto()) {
      Hide();
      event.Veto();
    } else event.Skip();
  });
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      auto newDefine = new CDefine(optionArea);
      customDefines.push_back(newDefine);
      newDefine->Bind(wxEVT_BUTTON, [newDefine, this](wxCommandEvent) {
          optionArea->GetSizer()->Detach(newDefine);
          customDefines.erase(std::find(customDefines.begin(), customDefines.end(), newDefine));
          newDefine->Destroy();
          updateOptions();
        }, CDefine::ID_Remove);
      updateOptions();
    }, ID_AddDefine);
}

void CustomOptionsDlg::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  createOptionArea();

  sizer->SetMinSize(450, 500);
  sizer->Add(header(), wxSizerFlags(0).Expand().Border(wxALL, 10));
  sizer->Add(optionArea, wxSizerFlags(1).Expand().Border(wxALL, 10));
  sizer->Add(info(), wxSizerFlags(0).Expand().Border(wxALL, 5));

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

wxBoxSizer* CustomOptionsDlg::info() {
  auto infoSizer = new wxBoxSizer(wxHORIZONTAL);

  auto text = new wxStaticText(this, wxID_ANY, "A full list of ProffieOS defines can be found ");
  auto link = new wxHyperlinkCtrl(this, wxID_ANY, "here.", "https://pod.hubbe.net/config/the-config_top-section.html");
  infoSizer->Add(text, wxSizerFlags(0).Center());
  infoSizer->Add(link);

  return infoSizer;
}

void CustomOptionsDlg::createOptionArea() {
  optionArea = new wxScrolledCanvas(this, wxID_ANY);
  auto sizer = new wxBoxSizer(wxVERTICAL);

  cricketsText = new wxStaticText(optionArea, wxID_ANY, "Once you add custom options they'll show up here.");
  sizer->Add(cricketsText, wxSizerFlags(1).Center());

  optionArea->SetScrollRate(0, 10);
  optionArea->SetSizerAndFit(sizer);
}

void CustomOptionsDlg::updateOptions() {
  optionArea->GetSizer()->Clear();

  cricketsText->Show(customDefines.empty());
  for (auto it = customDefines.begin(); it != customDefines.end();) {
    optionArea->GetSizer()->Add(*(it++), wxSizerFlags(0).Expand().Border(wxBOTTOM, 5));
  }
  optionArea->FitInside();

  Layout();
}

CustomOptionsDlg::CDefine::CDefine(wxScrolledCanvas* _parent) : wxWindow(_parent, wxID_ANY) {
  auto sizer = new wxBoxSizer(wxHORIZONTAL);

  defText = new wxStaticText(this, wxID_ANY, "#define");
  name = new pcTextCtrl(this, ID_Name);
  value = new pcTextCtrl(this, ID_Value);
  remove = new wxButton(this, ID_Remove, "Remove");

  sizer->Add(defText, wxSizerFlags(0).Center().Border(wxRIGHT, 5));
  sizer->Add(name, wxSizerFlags(2).Border(wxRIGHT, 5));
  sizer->Add(value, wxSizerFlags(3).Border(wxRIGHT, 5));
  sizer->Add(remove, wxSizerFlags(0).Border(wxRIGHT, 10));

  SetSizerAndFit(sizer);
}
