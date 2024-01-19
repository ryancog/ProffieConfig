// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "ui/pctextctrl.h"

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

class PresetsPage : public wxStaticBoxSizer {
public:
  PresetsPage(wxWindow*);

  void update();

  pcComboBox* bladeArray{nullptr};
  pcTextCtrl* styleInput{nullptr};
  wxListBox* presetList{nullptr};
  wxListBox* bladeList{nullptr};

  wxButton* addPreset{nullptr};
  wxButton* removePreset{nullptr};
  wxButton* movePresetUp{nullptr};
  wxButton* movePresetDown{nullptr};

  pcTextCtrl* nameInput{nullptr};
  pcTextCtrl* dirInput{nullptr};
  pcTextCtrl* trackInput{nullptr};

  struct PresetConfig {
    std::vector<wxString> styles{};
    wxString name{""};
    wxString dirs{""};
    wxString track{""};
  };

  enum {
    ID_BladeArray,
    ID_BladeList,
    ID_PresetList,
    ID_PresetChange,
    ID_AddPreset,
    ID_RemovePreset,
    ID_MovePresetUp,
    ID_MovePresetDown
  };

private:
  EditorWindow* parent{nullptr};

  void bindEvents();
  void createToolTips();

  wxBoxSizer* createPresetSelect();
  wxBoxSizer* createPresetConfig();

  void pushIfNewPreset();
  void rebuildBladeArrayList();
  void rebuildPresetList();
  void rebuildBladeList();
  void resizeAndFillPresets();
  void updateFields();

  void stripAndSaveEditor();
  void stripAndSaveName();
  void stripAndSaveDir();
  void stripAndSaveTrack();
};
