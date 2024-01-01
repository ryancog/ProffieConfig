// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/pages/presetspage.h"

#include "core/defines.h"
#include "core/utilities/misc.h"
#include "editor/editorwindow.h"
#include "editor/pages/bladearraypage.h"


#include <string>
#include <wx/tooltip.h>

PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, ""), parent(static_cast<EditorWindow*>(window)) {
  styleInput = new wxTextCtrl(GetStaticBox(), ID_PresetChange, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
  styleInput->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

  Add(createPresetConfig(), wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
  Add(createPresetSelect(), wxSizerFlags(/*proportion*/ 0).Border(wxTOP | wxRIGHT | wxBOTTOM, 10).Expand());
  Add(styleInput, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());

  bindEvents();
  createToolTips();
}

void PresetsPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) { parent->bladesPage->bladeArray->SetSelection(bladeArray->GetSelection()); update(); }, ID_BladeArray);

  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { parent->bladesPage->update(); update(); }, ID_BladeList);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { parent->bladesPage->update(); update(); }, ID_PresetList);

  GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); }, ID_PresetChange);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.push_back(PresetConfig());
        parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets[parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.size() - 1].name = "newpreset";

        parent->bladesPage->update();
        update();
      }, ID_AddPreset);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (presetList->GetSelection() >= 0) {
          parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.erase(std::next(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.begin(), parent->presetsPage->presetList->GetSelection()));

          parent->bladesPage->update();
          update();
        }
      }, ID_RemovePreset);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection());
        parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection()) = parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection() - 1);
        parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection() - 1) = tempStore;
        presetList->SetSelection(presetList->GetSelection() - 1);
        update();
      }, ID_MovePresetUp);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection());
        parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection()) = parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection() + 1);
        parent->bladeArrayPage->bladeArrays.at(bladeArray->GetSelection()).presets.at(presetList->GetSelection() + 1) = tempStore;
        presetList->SetSelection(presetList->GetSelection() + 1);
        update();
      }, ID_MovePresetDown);
}
void PresetsPage::createToolTips() {
  TIP(nameInput, "The name for the preset.\nThis value is typically just for reference, and doesn't mean anything, but if using an OLED and without a special bitmap, this name will be displayed.\nUsing \"\\n\" is like hitting \"enter\" when the text is displayed on the OLED.\nFor example, \"my\\npreset\" will be displayed on the OLED as two lines, the first being \"my\" and the second being \"preset\".");
  TIP(dirInput, "The path of the folder on the SD card where the font is stored.\nIf the font folder is inside another folder, it must be indicated by something like \"folderName/fontFolderName\".\nIn order to specify multiple directories (for example, to inlclude a \"common\" directory), use a semicolon (;) to seperate the folders (e.g. \"fontFolderName;common\").");
  TIP(trackInput, "The path of the track file on the SD card.\nIf the track is directly inside one of the folders specified in \"Font Directory\" then only the name of the track file is required.");

  TIP(bladeArray, "The currently-selected blade array to be edited.\nEach blade array has unique presets.");
  TIP(presetList, "All presets in this blade array.\nSelect a preset to edit associated blade styles.");
  TIP(bladeList, "All blades in this blade array.\nSelect a preset to edit associated blade styles.");

  TIP(addPreset, "Add a preset to the currently-selected blade array.");
  TIP(removePreset, "Delete the currently-selected preset.");
  
  TIP(styleInput, "Your blade style goes here.\nThis is the code which sets up what animations and effects your blade (or other LED) will do.\nFor getting/creating blade styles, see the Documentation (in \"Help->Documentation...\").");
}

wxBoxSizer* PresetsPage::createPresetSelect() {
  wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* arraySizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* bladeArrayText = new wxStaticText(GetStaticBox(), wxID_ANY, "Blade Array");
  bladeArray = new wxComboBox(GetStaticBox(), ID_BladeArray, "blade_in", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ "blade_in" }), wxCB_READONLY);
  arraySizer->Add(bladeArrayText, TEXTITEMFLAGS);
  arraySizer->Add(bladeArray, wxSizerFlags(0).Border(wxBOTTOM, 5).Expand());

  wxBoxSizer* arrangeButtonSizer = new wxBoxSizer(wxVERTICAL);
  movePresetUp = new wxButton(GetStaticBox(), ID_MovePresetUp, L"\u2191" /*up arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
  movePresetDown = new wxButton(GetStaticBox(), ID_MovePresetDown, L"\u2193" /*down arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
  arrangeButtonSizer->Add(movePresetUp, FIRSTITEMFLAGS);
  arrangeButtonSizer->Add(movePresetDown, MENUITEMFLAGS);
  wxBoxSizer* listSizer = new wxBoxSizer(wxHORIZONTAL);
  presetList = new wxListBox(GetStaticBox(), ID_PresetList);
  bladeList = new wxListBox(GetStaticBox(), ID_BladeList);
  listSizer->Add(arrangeButtonSizer, wxSizerFlags(0));
  listSizer->Add(presetList, wxSizerFlags(1).Expand());
  listSizer->Add(bladeList, wxSizerFlags(1).Expand());

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addPreset = new wxButton(GetStaticBox(), ID_AddPreset, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removePreset = new wxButton(GetStaticBox(), ID_RemovePreset, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addPreset, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 5));
  buttonSizer->Add(removePreset, wxSizerFlags(0).Border(wxTOP, 5));

  presetSelect->Add(arraySizer, wxSizerFlags(0).Expand().Border(wxLEFT, 25));
  presetSelect->Add(listSizer, wxSizerFlags(1).Expand());
  presetSelect->Add(buttonSizer, wxSizerFlags(0).Border(wxLEFT, 30));

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
  if (styleInput->IsModified()) stripAndSaveEditor();

  rebuildBladeArrayList();
  rebuildPresetList();
  rebuildBladeList();

  updateFields();
}
void PresetsPage::pushIfNewPreset() {
  if (presetList->GetSelection() == -1 && parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.size() > 0 && (!nameInput->IsEmpty() || !dirInput->IsEmpty() || !trackInput->IsEmpty())) {
    parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.push_back(PresetConfig());
    rebuildPresetList();
    presetList->SetSelection(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.size() - 1);
    bladeList->SetSelection(0);
  }
}
void PresetsPage::rebuildBladeArrayList() {
  int32_t arraySelection = bladeArray->GetSelection();
  bladeArray->Clear();
  for (const BladeArrayPage::BladeArray& array : parent->bladeArrayPage->bladeArrays) {
    bladeArray->Append(array.name);
  }
  if (arraySelection >= 0 && arraySelection < static_cast<int32_t>(bladeArray->GetCount())) bladeArray->SetSelection(arraySelection);
  else bladeArray->SetSelection(0);
}
void PresetsPage::rebuildPresetList() {
  int32_t listSelection = presetList->GetSelection();
  presetList->Clear();
  for (const PresetConfig& preset : parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets) {
    presetList->Append(preset.name);
  }
  if (static_cast<int32_t>(presetList->GetCount()) - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) presetList->SetSelection(listSelection);
}
void PresetsPage::rebuildBladeList() {
  int32_t listSelection = bladeList->GetSelection();
  bladeList->Clear();
  for (uint32_t blade = 0; blade < parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.size(); blade++) {
    if (parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.at(blade).subBlades.size() > 0) {
      for (uint32_t subBlade = 0; subBlade < parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.at(blade).subBlades.size(); subBlade++) {
        bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
      }
    } else {
      bladeList->Append("Blade " + std::to_string(blade));
    }
  }
  if (static_cast<int32_t>(bladeList->GetCount()) - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0) bladeList->SetSelection(listSelection);
}

void PresetsPage::resizeAndFillPresets() {
  auto getNumBlades = [&]() {
    int32_t numBlades = 0;
    for (const BladesPage::BladeConfig& blade : parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades) {
      numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
    }
    return numBlades;
  };

  for (PresetConfig& preset : parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets) {
    while (static_cast<int32_t>(preset.styles.size()) < getNumBlades()) {
      preset.styles.push_back("StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()");
    }
    while (static_cast<int32_t>(preset.styles.size()) > getNumBlades()) {
      preset.styles.pop_back();
    }
  }
}
void PresetsPage::updateFields() {
  if (presetList->GetSelection() >= 0) {
    uint32_t insertionPoint;
    
    insertionPoint = styleInput->GetInsertionPoint();
    if (bladeList->GetSelection() >= 0) {
      styleInput->ChangeValue(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()));
      styleInput->SetInsertionPoint(insertionPoint <= styleInput->GetValue().size() ? insertionPoint : styleInput->GetValue().size());
    } else {
      styleInput->ChangeValue("Select Blade to Edit Style...");
    }

    insertionPoint = nameInput->GetInsertionPoint();
    nameInput->ChangeValue(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).name);
    nameInput->SetInsertionPoint(insertionPoint <= nameInput->GetValue().size() ? insertionPoint : nameInput->GetValue().size());

    insertionPoint = dirInput->GetInsertionPoint();
    dirInput->ChangeValue(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).dirs);
    dirInput->SetInsertionPoint(insertionPoint <= dirInput->GetValue().size() ? insertionPoint : dirInput->GetValue().size());

    insertionPoint = trackInput->GetInsertionPoint();
    trackInput->ChangeValue(parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).track);
    trackInput->SetInsertionPoint(insertionPoint <= trackInput->GetValue().size() - 4 ? insertionPoint : trackInput->GetValue().size() - 4);
  }
  else {
    styleInput->ChangeValue("Select/Create Preset and Blade to Edit Style...");
    nameInput->ChangeValue("");
    dirInput->ChangeValue("");
    trackInput->ChangeValue("");
  }

  removePreset->Enable(presetList->GetSelection() != -1);
  movePresetDown->Enable(presetList->GetSelection() != -1 && presetList->GetSelection() < static_cast<int32_t>(presetList->GetCount()) - 1);
  movePresetUp->Enable(presetList->GetSelection() > 0);
  
  styleInput->SetModified(false); // Value is flagged as dirty from last change unless we manually reset it, causing overwrites where there shouldn't be.
  nameInput->SetModified(false);
  dirInput->SetModified(false);
  trackInput->SetModified(false);
}

void PresetsPage::stripAndSaveEditor() {
  if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
    wxString style = styleInput->GetValue();
    style.erase(std::remove(style.begin(), style.end(), ' '), style.end());
    if (style.find("{") != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '{'));
    if (style.rfind("}") != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '}'));
    if (style.rfind("(),") != wxString::npos) style.erase(style.rfind("(),") + 2);
    parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).styles.at(bladeList->GetSelection()).assign(style);
  }
}
void PresetsPage::stripAndSaveName() {
  if (presetList->GetSelection() >= 0 && parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    wxString name = nameInput->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); }); // to lowercase
    parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).name.assign(name);
  }
}
void PresetsPage::stripAndSaveDir() {
  if (presetList->GetSelection() >= 0 && parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    wxString dir = dirInput->GetValue();
    dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
    parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).dirs.assign(dir);
  }
}
void PresetsPage::stripAndSaveTrack() {
  wxString track = trackInput->GetValue();
  track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
  if (track.find(".") != wxString::npos) track.erase(track.find("."));
  if (track.length() > 0) track += ".wav";

  if (presetList->GetSelection() >= 0 && parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].blades.size() > 0) {
    parent->bladeArrayPage->bladeArrays[bladeArray->GetSelection()].presets.at(presetList->GetSelection()).track.assign(track);
  } else {
    trackInput->ChangeValue(track);
    trackInput->SetInsertionPoint(1);
  }
}
