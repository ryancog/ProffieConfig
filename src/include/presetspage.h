#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

#pragma once

class PresetsPage : public wxStaticBoxSizer {
public:
  PresetsPage(wxWindow*);
  static PresetsPage* instance;

  void update();

  wxComboBox* bladeArray{nullptr};
  wxTextCtrl* presetsEditor{nullptr};
  wxListBox* presetList{nullptr};
  wxListBox* bladeList{nullptr};
  wxButton* addPreset{nullptr};
  wxButton* removePreset{nullptr};

  wxTextCtrl* nameInput{nullptr};
  wxTextCtrl* dirInput{nullptr};
  wxTextCtrl* trackInput{nullptr};

  struct PresetConfig {
    std::vector<wxString> styles{};
    wxString name{""};
    wxString dirs{""};
    wxString track{""};
  };

private:
  PresetsPage();

  enum {
    ID_BladeArray,
    ID_BladeList,
    ID_PresetList,
    ID_PresetChange,
    ID_AddPreset,
    ID_RemovePreset
  };

  void bindEvents();

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

  bool selfEdit{false};
};
