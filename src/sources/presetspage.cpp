#include "presetspage.h"
#include "misc.h"
#include "configuration.h"
#include <string>

PresetsPage* PresetsPage::instance;
PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  instance = this;
  wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);
  settings.presetsEditor = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetEditor, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
  settings.presetsEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  wxBoxSizer *presetsConfig = new wxBoxSizer(wxVERTICAL);

  // Preset Select
  {
    wxBoxSizer *presetLists = new wxBoxSizer(wxHORIZONTAL);

    settings.presetList = new wxListBox(GetStaticBox(), Misc::ID_PresetList);
    settings.bladeList = new wxListBox(GetStaticBox(), Misc::ID_BladeList);
    presetLists->Add(settings.presetList, wxSizerFlags(1).Expand());
    presetLists->Add(settings.bladeList, wxSizerFlags(1).Expand());

    wxBoxSizer *presetButtons = new wxBoxSizer(wxHORIZONTAL);
    settings.addPreset = new wxButton(GetStaticBox(), Misc::ID_AddPreset, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    settings.removePreset = new wxButton(GetStaticBox(), Misc::ID_RemovePreset, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    presetButtons->Add(settings.addPreset, wxSizerFlags(0).Border(wxRIGHT, 10));
    presetButtons->Add(settings.removePreset);

    presetSelect->Add(presetLists, wxSizerFlags(1));
    presetSelect->Add(presetButtons, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 5));
  }

  // Preset Config
  {
    wxBoxSizer *name = new wxBoxSizer(wxVERTICAL);
    wxStaticText *nameLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Preset Name", wxDefaultPosition, wxSize(150, 20));
    settings.nameInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetName);
    name->Add(nameLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    name->Add(settings.nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

    wxBoxSizer *dir = new wxBoxSizer(wxVERTICAL);
    wxStaticText *dirLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
    settings.dirInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetDir);
    dir->Add(dirLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    dir->Add(settings.dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

    wxBoxSizer *track = new wxBoxSizer(wxVERTICAL);
    wxStaticText *trackLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
    settings.trackInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetTrack);
    track->Add(trackLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    track->Add(settings.trackInput, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM, 10).Expand());

    presetsConfig->Add(name);
    presetsConfig->Add(dir);
    presetsConfig->Add(track);
  }

  Add(presetsConfig, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
  Add(presetSelect, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10).Expand());
  Add(settings.presetsEditor, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
}

void PresetsPage::update() {
  int32_t presetIndex = settings.presetList->GetSelection();
  int32_t bladeIndex = settings.bladeList->GetSelection();

  if (presetIndex == -1 && Configuration::instance->blades.size() > 0 && (!settings.nameInput->IsEmpty() || !settings.dirInput->IsEmpty() || !settings.trackInput->IsEmpty())) {
    Configuration::instance->presets.push_back(Configuration::presetConfig());
    presetIndex = Configuration::instance->presets.size() - 1;

    Configuration::instance->presets.at(presetIndex).name = settings.nameInput->GetValue();
    Configuration::instance->presets.at(presetIndex).dirs = settings.dirInput->GetValue();
    Configuration::instance->presets.at(presetIndex).track = settings.trackInput->GetValue();
  }

  int32_t listSelection = presetIndex;
  settings.presetList->Clear();
  for (const Configuration::presetConfig& preset : Configuration::instance->presets) {
    settings.presetList->Append(preset.name);
  }
  if ((int32_t)settings.presetList->GetCount() - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) settings.presetList->SetSelection(listSelection);

  listSelection = bladeIndex;
  settings.bladeList->Clear();
  for (uint32_t blade = 0; blade < Configuration::instance->blades.size(); blade++) {
    if (Configuration::instance->blades[blade].subBlades.size() > 0) {
      for (uint32_t subBlade = 0; subBlade < Configuration::instance->blades[blade].subBlades.size(); subBlade++) {
        settings.bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
      }
    } else {
      settings.bladeList->Append("Blade " + std::to_string(blade));
    }
  }
  if ((int32_t)settings.bladeList->GetCount() - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) settings.bladeList->SetSelection(listSelection);

  for (Configuration::presetConfig& preset : Configuration::instance->presets) {
    // Calculate # of presets there should be prior.
    int32_t numBlades = 0;
    for (const Configuration::bladeConfig& blade : Configuration::instance->blades) {
      numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
    }

    if (numBlades > (int32_t)preset.styles.size()) {
      while (numBlades != (int32_t)preset.styles.size()) {
        preset.styles.push_back("StylePtr<Black>()");
      }
    } else if (numBlades < (int32_t)preset.styles.size()) {
      while (numBlades != (int32_t)preset.styles.size()) {
        preset.styles.pop_back();
      }
    }
  }

  presetIndex = settings.presetList->GetSelection();
  bladeIndex = settings.bladeList->GetSelection();
  if (presetIndex >= 0) {
    if (bladeIndex >= 0) settings.presetsEditor->ChangeValue(wxString::FromUTF8(Configuration::instance->presets[presetIndex].styles[bladeIndex]));
    else settings.presetsEditor->ChangeValue(wxString::FromUTF8("Select Blade to Edit Style..."));
    settings.nameInput->ChangeValue(wxString::FromUTF8(Configuration::instance->presets[presetIndex].name));
    settings.nameInput->SetInsertionPoint(settings.nameInput->GetValue().ToStdString().size());
    settings.dirInput->ChangeValue(wxString::FromUTF8(Configuration::instance->presets[presetIndex].dirs));
    settings.dirInput->SetInsertionPoint(settings.dirInput->GetValue().ToStdString().size());
    settings.trackInput->ChangeValue(wxString::FromUTF8(Configuration::instance->presets[presetIndex].track));
    settings.trackInput->SetInsertionPoint(settings.trackInput->GetValue().ToStdString().size());
  }
  else {
    settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));
    settings.nameInput->ChangeValue(wxString::FromUTF8(""));
    settings.dirInput->ChangeValue(wxString::FromUTF8(""));
    settings.trackInput->ChangeValue(wxString::FromUTF8(""));
  }

}

void PresetsPage::updatePresetEditor() {
  // Update Style Config
  if (PresetsPage::instance->settings.presetList->GetSelection() >= 0 && PresetsPage::instance->settings.bladeList->GetSelection() >= 0) {
    std::string style = PresetsPage::instance->settings.presetsEditor->GetValue().ToStdString();
    style.erase(std::remove(style.begin(), style.end(), ' '), style.end());
    Configuration::instance->presets[PresetsPage::instance->settings.presetList->GetSelection()].styles[PresetsPage::instance->settings.bladeList->GetSelection()].assign(style);
  }

  Configuration::instance->updateBladesConfig();
  PresetsPage::instance->update();
}
void PresetsPage::updatePresetName() {
  // Update Name Config
  if (PresetsPage::instance->settings.presetList->GetSelection() >= 0 && Configuration::instance->blades.size() > 0) {
    std::string name = PresetsPage::instance->settings.nameInput->GetValue().ToStdString();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    Configuration::instance->presets[PresetsPage::instance->settings.presetList->GetSelection()].name.assign(name);
  }

  Configuration::instance->updateBladesConfig(); PresetsPage::instance->update();
}
void PresetsPage::updatePresetDir() {
  // Update Dir Config
  if (PresetsPage::instance->settings.presetList->GetSelection() >= 0 && Configuration::instance->blades.size() > 0) {
    std::string dir =  PresetsPage::instance->settings.dirInput->GetValue().ToStdString();
    dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
    Configuration::instance->presets[PresetsPage::instance->settings.presetList->GetSelection()].dirs.assign(dir);
  }

  Configuration::instance->updateBladesConfig(); PresetsPage::instance->update();

}
void PresetsPage::updatePresetTrack() {
  // Update Track Config
  if (PresetsPage::instance->settings.presetList->GetSelection() >= 0 && Configuration::instance->blades.size() > 0) {
    std::string track = PresetsPage::instance->settings.trackInput->GetValue().ToStdString();
    track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
    Configuration::instance->presets[PresetsPage::instance->settings.presetList->GetSelection()].track.assign(track);
  }

  Configuration::instance->updateBladesConfig(); PresetsPage::instance->update();
}
