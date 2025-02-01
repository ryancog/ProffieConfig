// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "editor/pages/presetspage.h"

#include "core/defines.h"
#include "core/utilities/misc.h"
#include "editor/editorwindow.h"
#include "editor/dialogs/bladearraydlg.h"

#include <string>
#include <wx/tooltip.h>
#ifdef __WXGTK__
#include <wx/clipbrd.h>
#endif

PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, ""), parent(static_cast<EditorWindow*>(window)) {
  commentInput = new pcTextCtrl(GetStaticBox(), ID_PresetChange, "Comments", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE | wxNO_BORDER);
  styleInput = new pcTextCtrl(GetStaticBox(), ID_PresetChange, "BladeStyle", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE | wxNO_BORDER);
  styleInput->entry()->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));


  Add(createPresetConfig(), wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
  Add(createPresetSelect(), wxSizerFlags(/*proportion*/ 0).Border(wxTOP | wxRIGHT | wxBOTTOM, 10).Expand());

  auto *styleSizer{new wxBoxSizer(wxVERTICAL)};
  styleSizer->Add(commentInput, wxSizerFlags(/*proportion*/ 1).Expand());
  styleSizer->AddSpacer(10);
  styleSizer->Add(styleInput, wxSizerFlags(/*proportion*/ 2).Expand());
  Add(styleSizer, wxSizerFlags(1).Border(wxALL, 10).Expand());

  bindEvents();
  createToolTips();
}

void PresetsPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { parent->bladesPage->bladeArray->entry()->SetSelection(bladeArray->entry()->GetSelection()); update(); }, ID_BladeArray);

  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { parent->bladesPage->update(); update(); }, ID_BladeList);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { parent->bladesPage->update(); update(); }, ID_PresetList);

  GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); }, ID_PresetChange);
# ifdef __WXGTK__ // GTK processes double events, leading to crash it seems... this is a workaround.
  GetStaticBox()->Bind(wxEVT_TEXT_PASTE, [&](wxClipboardTextEvent&) {
        if (!wxClipboard::Get()->Open()) return;

        wxTextDataObject data;
        wxClipboard::Get()->GetData(data);
        wxClipboard::Get()->Close();

        auto styleText = styleInput->entry()->GetValue();
        long selectionStart, selectionEnd;
        styleInput->entry()->GetSelection(&selectionStart, &selectionEnd);

        if (selectionStart != selectionEnd) styleText.erase(selectionStart, selectionEnd);
        styleText.insert(selectionStart, data.GetText());

        styleInput->entry()->SetModified(true);
        styleInput->entry()->SetValue(styleText);
        styleInput->entry()->SetInsertionPoint(selectionStart + data.GetText().size());
  }, ID_PresetChange);
# endif
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.push_back(PresetConfig());
        parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets[parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.size() - 1].name = "newpreset";

        parent->bladesPage->update();
        update();
      }, ID_AddPreset);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (presetList->GetSelection() >= 0) {
          parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.erase(std::next(parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.begin(), parent->presetsPage->presetList->GetSelection()));

          parent->bladesPage->update();
          update();
        }
      }, ID_RemovePreset);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection());
        parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection()) = parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection() - 1);
        parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection() - 1) = tempStore;
        presetList->SetSelection(presetList->GetSelection() - 1);
        update();
      }, ID_MovePresetUp);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection());
        parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection()) = parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection() + 1);
        parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection() + 1) = tempStore;
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
  
  TIP(styleInput, "Any comments about the blade style goes here.\nThis doesn't affect the blade style at all, but can be a place for helpful notes!");
  TIP(styleInput, "Your blade style goes here.\nThis is the code which sets up what animations and effects your blade (or other LED) will do.\nFor getting/creating blade styles, see the Documentation (in \"Help->Documentation...\").");
}

wxBoxSizer* PresetsPage::createPresetSelect() {
  wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* arraySizer = new wxBoxSizer(wxVERTICAL);
  bladeArray = new pcChoice(GetStaticBox(), ID_BladeArray, "Blade Array", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ "blade_in" }), 0);
  arraySizer->Add(bladeArray, wxSizerFlags(0).Border(wxBOTTOM, 5).Expand());

  auto *listSizer{new wxBoxSizer(wxHORIZONTAL)};
  auto *arrangeButtonSizer{new wxBoxSizer(wxVERTICAL)};
  movePresetUp = new wxButton(GetStaticBox(), ID_MovePresetUp, L"\u2191" /*up arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
  movePresetDown = new wxButton(GetStaticBox(), ID_MovePresetDown, L"\u2193" /*down arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
  arrangeButtonSizer->Add(movePresetUp, FIRSTITEMFLAGS);
  arrangeButtonSizer->Add(movePresetDown, MENUITEMFLAGS);
  listSizer->Add(arrangeButtonSizer, wxSizerFlags(0));
  auto *presetSizer{new wxBoxSizer(wxVERTICAL)};
  presetList = new wxListBox(GetStaticBox(), ID_PresetList, wxDefaultPosition, wxDefaultSize, wxArrayString{}, wxBORDER_NONE);
  presetSizer->Add(new wxStaticText(GetStaticBox(), wxID_ANY, "Presets"), wxSizerFlags());
  presetSizer->Add(presetList, wxSizerFlags(1).Expand());
  auto *bladeSizer{new wxBoxSizer(wxVERTICAL)};
  bladeList = new wxListBox(GetStaticBox(), ID_BladeList, wxDefaultPosition, wxDefaultSize, wxArrayString{}, wxBORDER_NONE);
  bladeSizer->Add(new wxStaticText(GetStaticBox(), wxID_ANY, "Blades"), wxSizerFlags());
  bladeSizer->Add(bladeList, wxSizerFlags(1).Expand());
  listSizer->Add(presetSizer, wxSizerFlags(1).Expand().Border(wxALL, 4));
  listSizer->Add(bladeSizer, wxSizerFlags(1).Expand().Border(wxALL, 4));

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
  nameInput = new pcTextCtrl(GetStaticBox(), ID_PresetChange);
  name->Add(nameLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
  name->Add(nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

  wxBoxSizer *dir = new wxBoxSizer(wxVERTICAL);
  wxStaticText *dirLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
  dirInput = new pcTextCtrl(GetStaticBox(), ID_PresetChange);
  dir->Add(dirLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
  dir->Add(dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

  wxBoxSizer *track = new wxBoxSizer(wxVERTICAL);
  wxStaticText *trackLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
  trackInput = new pcTextCtrl(GetStaticBox(), ID_PresetChange);
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

  if (nameInput->entry()->IsModified()) stripAndSaveName();
  if (dirInput->entry()->IsModified()) stripAndSaveDir();
  if (trackInput->entry()->IsModified()) stripAndSaveTrack();
  if (styleInput->entry()->IsModified()) stripAndSaveEditor();
  if (commentInput->entry()->IsModified()) stripAndSaveComments();

  rebuildBladeArrayList();
  rebuildPresetList();
  rebuildBladeList();

  updateFields();
}
void PresetsPage::pushIfNewPreset() {
  if (presetList->GetSelection() == -1 && parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 0 && (!nameInput->entry()->IsEmpty() || !dirInput->entry()->IsEmpty() || !trackInput->entry()->IsEmpty())) {
    parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.push_back(PresetConfig());
    rebuildPresetList();
    presetList->SetSelection(parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.size() - 1);
    bladeList->SetSelection(0);
  }
}
void PresetsPage::rebuildBladeArrayList() {
  int32_t arraySelection = bladeArray->entry()->GetSelection();
  bladeArray->entry()->Clear();
  for (const BladeArrayDlg::BladeArray& array : parent->bladesPage->bladeArrayDlg->bladeArrays) {
    bladeArray->entry()->Append(array.name);
  }
  if (arraySelection >= 0 && arraySelection < static_cast<int32_t>(bladeArray->entry()->GetCount())) bladeArray->entry()->SetSelection(arraySelection);
  else bladeArray->entry()->SetSelection(0);
}
void PresetsPage::rebuildPresetList() {
  int32_t listSelection = presetList->GetSelection();
  presetList->Clear();
  for (const PresetConfig& preset : parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets) {
    presetList->Append(preset.name);
  }
  if (static_cast<int32_t>(presetList->GetCount()) - 1 < listSelection) listSelection -= 1;
  if (listSelection >= 0 && listSelection < static_cast<int32_t>(presetList->GetCount())) presetList->SetSelection(listSelection);
  else if (presetList->GetCount()) presetList->SetSelection(0);
}
void PresetsPage::rebuildBladeList() {
  int32_t listSelection = bladeList->GetSelection();
  bladeList->Clear();
  for (uint32_t blade = 0; blade < parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size(); blade++) {
    if (parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(blade).subBlades.size() > 0) {
      for (uint32_t subBlade = 0; subBlade < parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.at(blade).subBlades.size(); subBlade++) {
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
    for (const BladesPage::BladeConfig& blade : parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades) {
      numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
    }
    return numBlades;
  };

  for (PresetConfig& preset : parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets) {
    while (static_cast<int32_t>(preset.styles.size()) < getNumBlades()) {
        preset.styles.push_back({ {}, "StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()" });
    }
    while (static_cast<int32_t>(preset.styles.size()) > getNumBlades()) {
      preset.styles.pop_back();
    }
  }
}
void PresetsPage::updateFields() {
    if (presetList->GetSelection() >= 0) {
        const auto& currentPreset = parent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArray->entry()->GetSelection()).presets.at(presetList->GetSelection());
        uint32_t insertionPoint;

        if (bladeList->GetSelection() >= 0) {
            const auto& style{currentPreset.styles.at(bladeList->GetSelection())};

            insertionPoint = styleInput->entry()->GetInsertionPoint();
            styleInput->entry()->ChangeValue(style.style);
            styleInput->entry()->SetInsertionPoint(std::min<uint32_t>(insertionPoint, styleInput->entry()->GetLastPosition()));

            insertionPoint = commentInput->entry()->GetInsertionPoint();
            commentInput->entry()->ChangeValue(style.comment);
            commentInput->entry()->SetInsertionPoint(std::min<uint32_t>(insertionPoint, commentInput->entry()->GetLastPosition()));
        } else {
            commentInput->entry()->ChangeValue("Select Blade to Edit Style Comments...");
            styleInput->entry()->ChangeValue("Select Blade to Edit Style...");
        }

        insertionPoint = nameInput->entry()->GetInsertionPoint();
        nameInput->entry()->ChangeValue(currentPreset.name);
        nameInput->entry()->SetInsertionPoint(insertionPoint <= nameInput->entry()->GetValue().size() ? insertionPoint : nameInput->entry()->GetValue().size());

        insertionPoint = dirInput->entry()->GetInsertionPoint();
        dirInput->entry()->ChangeValue(currentPreset.dirs);
        dirInput->entry()->SetInsertionPoint(insertionPoint <= dirInput->entry()->GetValue().size() ? insertionPoint : dirInput->entry()->GetValue().size());

        insertionPoint = trackInput->entry()->GetInsertionPoint();
        trackInput->entry()->ChangeValue(currentPreset.track);
        trackInput->entry()->SetInsertionPoint(insertionPoint <= trackInput->entry()->GetValue().size() - 4 ? insertionPoint : trackInput->entry()->GetValue().size() - 4);
    }
    else {
        commentInput->entry()->ChangeValue("Select/Create Preset and Blade to Edit Style Comments...");
        styleInput->entry()->ChangeValue("Select/Create Preset and Blade to Edit Style...");
        nameInput->entry()->ChangeValue("");
        dirInput->entry()->ChangeValue("");
        trackInput->entry()->ChangeValue("");
    }

    commentInput->entry()->Enable(presetList->GetSelection() != -1 and bladeList->GetSelection() != -1);
    styleInput->entry()->Enable(presetList->GetSelection() != -1 and bladeList->GetSelection() != -1);
    removePreset->Enable(presetList->GetSelection() != -1);
    movePresetDown->Enable(presetList->GetSelection() != -1 && presetList->GetSelection() < static_cast<int32_t>(presetList->GetCount()) - 1);
    movePresetUp->Enable(presetList->GetSelection() > 0);

    // Value is flagged as dirty from last change unless we manually reset it, causing overwrites where there shouldn't be.
    styleInput->entry()->SetModified(false);
    commentInput->entry()->SetModified(false);
    nameInput->entry()->SetModified(false);
    dirInput->entry()->SetModified(false);
    trackInput->entry()->SetModified(false);
}

void PresetsPage::stripAndSaveComments() {
    if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
        auto comments{commentInput->entry()->GetValue()};

        size_t illegalStrPos{0};
        while ((illegalStrPos = comments.find("/*")) != std::string::npos) comments.erase(illegalStrPos, 2);
        while ((illegalStrPos = comments.find("*/")) != std::string::npos) comments.erase(illegalStrPos, 2);
        while ((illegalStrPos = comments.find("//")) != std::string::npos) comments.erase(illegalStrPos, 2);

        auto& selectedBladeArray{parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()]};
        auto& selectedPreset{selectedBladeArray.presets[presetList->GetSelection()]};

        selectedPreset.styles[bladeList->GetSelection()].comment = comments;
    }
}

void PresetsPage::stripAndSaveEditor() {
  if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
    wxString style = styleInput->entry()->GetValue();
    if (style.find('{') != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '{'));
    if (style.rfind('}') != wxString::npos) style.erase(std::remove(style.begin(), style.end(), '}'));
    if (style.rfind(")") != wxString::npos) style.erase(style.rfind(")") + 1);

    auto& selectedBladeArray{parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()]};
    auto& selectedPreset{selectedBladeArray.presets[presetList->GetSelection()]};

    selectedPreset.styles[bladeList->GetSelection()].style = style;
  }
}
void PresetsPage::stripAndSaveName() {
  if (presetList->GetSelection() >= 0 && parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 0) {
    wxString name = nameInput->entry()->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); }); // to lowercase
    parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.at(presetList->GetSelection()).name.assign(name);
  }
}
void PresetsPage::stripAndSaveDir() {
  if (presetList->GetSelection() >= 0 && parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 0) {
    wxString dir = dirInput->entry()->GetValue();
    // dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
    parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.at(presetList->GetSelection()).dirs.assign(dir);
  }
}
void PresetsPage::stripAndSaveTrack() {
  wxString track = trackInput->entry()->GetValue();
  track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
  if (track.find(".") != wxString::npos) track.erase(track.find("."));
  if (track.length() > 0) track += ".wav";

  if (presetList->GetSelection() >= 0 && parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].blades.size() > 0) {
    parent->bladesPage->bladeArrayDlg->bladeArrays[bladeArray->entry()->GetSelection()].presets.at(presetList->GetSelection()).track.assign(track);
  } else {
    trackInput->entry()->ChangeValue(track);
    trackInput->entry()->SetInsertionPoint(1);
  }
}
