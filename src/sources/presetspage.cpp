#include "presetspage.h"

#include "defines.h"
#include "misc.h"
#include "bladeidpage.h"

#include <string>

PresetsPage* PresetsPage::instance;
PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  instance = this;

  presetsEditor = new wxTextCtrl(GetStaticBox(), ID_PresetChange, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
  presetsEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

  Add(createPresetConfig(), wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
  Add(createPresetSelect(), wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10).Expand());
  Add(presetsEditor, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());

  bindEvents();
}

void PresetsPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { BladesPage::instance->bladeArray->SetSelection(bladeArray->GetSelection()); update(); }, ID_BladeArray);

  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { BladesPage::instance->update(); update(); }, ID_BladeList);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { BladesPage::instance->update(); update(); }, ID_PresetList);

  GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); }, ID_PresetChange);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.push_back(PresetConfig());
        BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets[BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.size() - 1].name = "newpreset";

        BladesPage::instance->update();
        update();
      }, ID_AddPreset);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (presetList->GetSelection() >= 0) {
          BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.erase(std::next(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.begin(), PresetsPage::instance->presetList->GetSelection()));

          BladesPage::instance->update();
          update();
        }
      }, ID_RemovePreset);

}

wxBoxSizer* PresetsPage::createPresetSelect() {
  wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* arraySizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* bladeArrayText = new wxStaticText(GetStaticBox(), wxID_ANY, "Blade Array");
  bladeArray = new wxComboBox(GetStaticBox(), ID_BladeArray, "blade_in", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ "blade_in" }), wxCB_READONLY);
  arraySizer->Add(bladeArrayText, TEXTITEMFLAGS);
  arraySizer->Add(bladeArray, wxSizerFlags(0).Border(wxBOTTOM, 5).Expand());

  wxBoxSizer* listSizer = new wxBoxSizer(wxHORIZONTAL);
  presetList = new wxListBox(GetStaticBox(), ID_PresetList);
  bladeList = new wxListBox(GetStaticBox(), ID_BladeList);
  listSizer->Add(presetList, wxSizerFlags(1).Expand());
  listSizer->Add(bladeList, wxSizerFlags(1).Expand());

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addPreset = new wxButton(GetStaticBox(), ID_AddPreset, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removePreset = new wxButton(GetStaticBox(), ID_RemovePreset, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addPreset, wxSizerFlags(0).Border(wxRIGHT, 10));
  buttonSizer->Add(removePreset);

  presetSelect->Add(arraySizer, wxSizerFlags(0).Expand());
  presetSelect->Add(listSizer, wxSizerFlags(1).Expand());
  presetSelect->Add(buttonSizer, wxSizerFlags(0).Border(wxTOP, 5));

  return presetSelect;
}
wxBoxSizer* PresetsPage::createPresetConfig() {
  wxBoxSizer *presetConfig = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *name = new wxBoxSizer(wxVERTICAL);
  wxStaticText *nameLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Preset Name", wxDefaultPosition, wxSize(150, 20));
  nameInput = new wxTextCtrl(GetStaticBox(), ID_PresetChange);
  name->Add(nameLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
  name->Add(nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

  wxBoxSizer *dir = new wxBoxSizer(wxVERTICAL);
  wxStaticText *dirLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
  dirInput = new wxTextCtrl(GetStaticBox(), ID_PresetChange);
  dir->Add(dirLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
  dir->Add(dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

  wxBoxSizer *track = new wxBoxSizer(wxVERTICAL);
  wxStaticText *trackLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
  trackInput = new wxTextCtrl(GetStaticBox(), ID_PresetChange);
  track->Add(trackLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
  track->Add(trackInput, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM, 10).Expand());

  presetConfig->Add(name);
  presetConfig->Add(dir);
  presetConfig->Add(track);


  return presetConfig;
}

void PresetsPage::update() {
  pushIfNewPreset();
  resizeAndFillPresets();

  if (nameInput->IsModified()) stripAndSaveName();
  if (dirInput->IsModified()) stripAndSaveDir();
  if (trackInput->IsModified()) stripAndSaveTrack();
  if (presetsEditor->IsModified()) stripAndSaveEditor();

  rebuildBladeArrayList();
  rebuildPresetList();
  rebuildBladeList();

  updateFields();
}
void PresetsPage::pushIfNewPreset() {
  if (presetList->GetSelection() == -1 && BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() > 0 && (!nameInput->IsEmpty() || !dirInput->IsEmpty() || !trackInput->IsEmpty())) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.push_back(PresetConfig());
    rebuildPresetList();
    presetList->SetSelection(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.size() - 1);
    bladeList->SetSelection(0);
  }
}
void PresetsPage::rebuildBladeArrayList() {
  int32_t arraySelection = bladeArray->GetSelection();
  bladeArray->Clear();
  for (const BladeIDPage::BladeArray& array : BladeIDPage::instance->bladeArrays) {
    bladeArray->Append(array.name);
  }
  if (arraySelection >= 0 && arraySelection < static_cast<int32_t>(bladeArray->GetCount())) bladeArray->SetSelection(arraySelection);
  else bladeArray->SetSelection(0);
}
void PresetsPage::rebuildPresetList() {
  int32_t listSelection = presetList->GetSelection();
  presetList->Clear();
  for (const PresetConfig& preset : BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets) {
    presetList->Append(preset.name);
  }
  if (static_cast<int32_t>(presetList->GetCount()) - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) presetList->SetSelection(listSelection);
}
void PresetsPage::rebuildBladeList() {
  int32_t listSelection = bladeList->GetSelection();
  bladeList->Clear();
  for (uint32_t blade = 0; blade < BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size(); blade++) {
    if (BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(blade).subBlades.size() > 0) {
      for (uint32_t subBlade = 0; subBlade < BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.at(blade).subBlades.size(); subBlade++) {
        bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
      }
    } else {
      bladeList->Append("Blade " + std::to_string(blade));
    }
  }
  if (static_cast<int32_t>(bladeList->GetCount()) - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) bladeList->SetSelection(listSelection);
}
int32_t PresetsPage::getNumBlades() {
  int32_t numBlades = 0;
  for (const BladesPage::BladeConfig& blade : BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades) {
    numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
  }
  return numBlades;
}
void PresetsPage::resizeAndFillPresets() {
  for (PresetConfig& preset : BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets) {
    while (static_cast<int32_t>(preset.styles.size()) < getNumBlades()) {
      preset.styles.push_back("StylePtr<Black>()");
    }
    while (static_cast<int32_t>(preset.styles.size()) > getNumBlades()) {
      preset.styles.pop_back();
    }
  }
}
void PresetsPage::updateFields() {
  if (presetList->GetSelection() >= 0) {
    uint32_t insertionPoint;

    insertionPoint = presetsEditor->GetInsertionPoint();
    if (bladeList->GetSelection() >= 0) {
      presetsEditor->ChangeValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()));
      presetsEditor->SetInsertionPoint(insertionPoint <= presetsEditor->GetValue().size() ? insertionPoint : presetsEditor->GetValue().size());
    } else {
      presetsEditor->ChangeValue("Select Blade to Edit Style...");
    }

    insertionPoint = nameInput->GetInsertionPoint();
    nameInput->ChangeValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).name);
    nameInput->SetInsertionPoint(insertionPoint <= nameInput->GetValue().size() ? insertionPoint : nameInput->GetValue().size());

    insertionPoint = dirInput->GetInsertionPoint();
    dirInput->ChangeValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).dirs);
    dirInput->SetInsertionPoint(insertionPoint <= dirInput->GetValue().size() ? insertionPoint : dirInput->GetValue().size());

    insertionPoint = trackInput->GetInsertionPoint();
    trackInput->ChangeValue(BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).track);
    trackInput->SetInsertionPoint(insertionPoint <= trackInput->GetValue().size() - 4 ? insertionPoint : trackInput->GetValue().size() - 4);
  }
  else {
    presetsEditor->ChangeValue("Select/Create Preset and Blade to Edit Style...");
    nameInput->ChangeValue("");
    dirInput->ChangeValue("");
    trackInput->ChangeValue("");
  }

  presetsEditor->SetModified(false); // Value is flagged as dirty from last change unless we manually reset it, causing overwrites where there shouldn't be.
  nameInput->SetModified(false);
  dirInput->SetModified(false);
  trackInput->SetModified(false);
}

void PresetsPage::stripAndSaveEditor() {
  if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
    wxString style = presetsEditor->GetValue();
    style.erase(std::remove(style.begin(), style.end(), ' '), style.end());
    if (style.find("{") != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '{'));
    if (style.rfind("}") != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '}'));
    if (style.rfind("(),") != wxString::npos) style.erase(style.rfind("(),") + 2);
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()).assign(style);
  }
}
void PresetsPage::stripAndSaveName() {
  if (presetList->GetSelection() >= 0 && BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    wxString name = nameInput->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); }); // to lowercase
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).name.assign(name);
  }
}
void PresetsPage::stripAndSaveDir() {
  if (presetList->GetSelection() >= 0 && BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    wxString dir = dirInput->GetValue();
    dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).dirs.assign(dir);
  }
}
void PresetsPage::stripAndSaveTrack() {
  wxString track = trackInput->GetValue();
  track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
  if (track.find(".") != wxString::npos) track.erase(track.find("."));
  if (track.length() > 0) track += ".wav";

  if (presetList->GetSelection() >= 0 && BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    BladeIDPage::instance->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).track.assign(track);
  } else {
    trackInput->ChangeValue(track);
    trackInput->SetInsertionPoint(1);
  }
}
