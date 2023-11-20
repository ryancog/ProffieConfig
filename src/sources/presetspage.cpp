#include "presetspage.h"

#include "misc.h"
#include "bladespage.h"

#include <string>

PresetsPage* PresetsPage::instance;
PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  instance = this;

  wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);
  presetsEditor = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetChange, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
  presetsEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  wxBoxSizer *presetsConfig = new wxBoxSizer(wxVERTICAL);

  // Preset Select
  {
    //bladeArraySelection = new wxComboBox(GetStaticBox(), Misc::ID_PresetChange, "0", wxDefaultPosition, wxDefaultSize, {"0", "100"}, wxCB_READONLY | wxCB_SORT);

    wxBoxSizer *presetLists = new wxBoxSizer(wxHORIZONTAL);

    presetList = new wxListBox(GetStaticBox(), Misc::ID_PresetList);
    bladeList = new wxListBox(GetStaticBox(), Misc::ID_BladeList);
    presetLists->Add(presetList, wxSizerFlags(1).Expand());
    presetLists->Add(bladeList, wxSizerFlags(1).Expand());

    wxBoxSizer *presetButtons = new wxBoxSizer(wxHORIZONTAL);
    addPreset = new wxButton(GetStaticBox(), Misc::ID_AddPreset, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    removePreset = new wxButton(GetStaticBox(), Misc::ID_RemovePreset, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    presetButtons->Add(addPreset, wxSizerFlags(0).Border(wxRIGHT, 10));
    presetButtons->Add(removePreset);

    //presetSelect->Add(new wxStaticText(GetStaticBox(), wxID_ANY, "Blade Array"));
    //presetSelect->Add(bladeArraySelection, wxSizerFlags(0).Border(wxBOTTOM, 5).Expand());
    presetSelect->Add(presetLists, wxSizerFlags(1));
    presetSelect->Add(presetButtons, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 5));
  }

  // Preset Config
  {
    wxBoxSizer *name = new wxBoxSizer(wxVERTICAL);
    wxStaticText *nameLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Preset Name", wxDefaultPosition, wxSize(150, 20));
    nameInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetChange);
    name->Add(nameLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    name->Add(nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

    wxBoxSizer *dir = new wxBoxSizer(wxVERTICAL);
    wxStaticText *dirLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
    dirInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetChange);
    dir->Add(dirLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    dir->Add(dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

    wxBoxSizer *track = new wxBoxSizer(wxVERTICAL);
    wxStaticText *trackLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
    trackInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetChange);
    track->Add(trackLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
    track->Add(trackInput, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM, 10).Expand());

    presetsConfig->Add(name);
    presetsConfig->Add(dir);
    presetsConfig->Add(track);
  }

  Add(presetsConfig, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
  Add(presetSelect, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10).Expand());
  Add(presetsEditor, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
}

void PresetsPage::update() {
  pushIfNewPreset();
  resizeAndFillPresets();

  if (nameInput->IsModified()) stripAndSaveName();
  if (dirInput->IsModified()) stripAndSaveDir();
  if (trackInput->IsModified()) stripAndSaveTrack();
  if (presetsEditor->IsModified()) stripAndSaveEditor();

  rebuildPresetList();
  rebuildBladeList();

  updateFields();
}
void PresetsPage::pushIfNewPreset() {
  if (presetList->GetSelection() == -1 && BladesPage::instance->blades.size() > 0 && (!nameInput->IsEmpty() || !dirInput->IsEmpty() || !trackInput->IsEmpty())) {
    presets.push_back(presetConfig());
    rebuildPresetList();
    presetList->SetSelection(presets.size() - 1);
    bladeList->SetSelection(0);
  }
}
void PresetsPage::rebuildPresetList() {
  int32_t listSelection = presetList->GetSelection();
  presetList->Clear();
  for (const presetConfig& preset : presets) {
    presetList->Append(preset.name);
  }
  if ((int32_t)presetList->GetCount() - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) presetList->SetSelection(listSelection);
}
void PresetsPage::rebuildBladeList() {
  int32_t listSelection = bladeList->GetSelection();
  bladeList->Clear();
  for (uint32_t blade = 0; blade < BladesPage::instance->blades.size(); blade++) {
    if (BladesPage::instance->blades.at(blade).subBlades.size() > 0) {
      for (uint32_t subBlade = 0; subBlade < BladesPage::instance->blades.at(blade).subBlades.size(); subBlade++) {
        bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
      }
    } else {
      bladeList->Append("Blade " + std::to_string(blade));
    }
  }
  if ((int32_t)bladeList->GetCount() - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) bladeList->SetSelection(listSelection);
}
int PresetsPage::getNumBlades() {
  int32_t numBlades = 0;
  for (const BladesPage::bladeConfig& blade : BladesPage::instance->blades) {
    numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
  }
  return numBlades;
}
void PresetsPage::resizeAndFillPresets() {
  for (presetConfig& preset : presets) {
    while ((int32_t)preset.styles.size() < getNumBlades()) {
      preset.styles.push_back("StylePtr<Black>()");
    }
    while ((int32_t)preset.styles.size() > getNumBlades()) {
      preset.styles.pop_back();
    }
  }
}
void PresetsPage::updateFields() {
  if (presetList->GetSelection() >= 0) {
    uint32_t insertionPoint;

    insertionPoint = presetsEditor->GetInsertionPoint();
    if (bladeList->GetSelection() >= 0) {
      presetsEditor->ChangeValue(presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()));
      presetsEditor->SetInsertionPoint(insertionPoint <= presetsEditor->GetValue().size() ? insertionPoint : presetsEditor->GetValue().size());
    } else presetsEditor->ChangeValue("Select Blade to Edit Style...");

    insertionPoint = nameInput->GetInsertionPoint();
    nameInput->ChangeValue(presets.at(presetList->GetSelection()).name);
    nameInput->SetInsertionPoint(insertionPoint <= nameInput->GetValue().size() ? insertionPoint : nameInput->GetValue().size());

    insertionPoint = dirInput->GetInsertionPoint();
    dirInput->ChangeValue(presets.at(presetList->GetSelection()).dirs);
    dirInput->SetInsertionPoint(insertionPoint <= dirInput->GetValue().size() ? insertionPoint : dirInput->GetValue().size());

    insertionPoint = trackInput->GetInsertionPoint();
    trackInput->ChangeValue(presets.at(presetList->GetSelection()).track);
    trackInput->SetInsertionPoint(insertionPoint <= trackInput->GetValue().size() - 4 ? insertionPoint : trackInput->GetValue().size() - 4);
  }
  else {
    presetsEditor->ChangeValue("Select/Create Preset and Blade to Edit Style...");
    nameInput->ChangeValue("");
    dirInput->ChangeValue("");
    trackInput->ChangeValue("");
  }
}

void PresetsPage::stripAndSaveEditor() {
  if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
    wxString style = presetsEditor->GetValue();
    style.erase(std::remove(style.begin(), style.end(), ' '), style.end());
    if (style.rfind("(),") != wxString::npos) style.erase(style.rfind("(),") + 2);
    presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()).assign(style);
  }
}
void PresetsPage::stripAndSaveName() {
  if (PresetsPage::instance->presetList->GetSelection() >= 0 && BladesPage::instance->blades.size() > 0) {
    wxString name = PresetsPage::instance->nameInput->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    presets.at(presetList->GetSelection()).name.assign(name);
  }
}
void PresetsPage::stripAndSaveDir() {
  if (presetList->GetSelection() >= 0 && BladesPage::instance->blades.size() > 0) {
    wxString dir = dirInput->GetValue();
    dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
    presets.at(presetList->GetSelection()).dirs.assign(dir);
  }
}
void PresetsPage::stripAndSaveTrack() {
  wxString track = trackInput->GetValue();
  track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
  if (track.find(".") != wxString::npos) track.erase(track.find("."));
  if (track.length() > 0) track += ".wav";

  if (presetList->GetSelection() >= 0 && BladesPage::instance->blades.size() > 0) {
    presets.at(presetList->GetSelection()).track.assign(track);
  } else {
    trackInput->ChangeValue(track);
    trackInput->SetInsertionPoint(1);
  }
}
