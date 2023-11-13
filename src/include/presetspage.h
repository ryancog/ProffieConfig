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

  wxTextCtrl* presetsEditor{nullptr};
  wxListBox* presetList{nullptr};
  wxListBox* bladeList{nullptr};
  wxButton* addPreset{nullptr};
  wxButton* removePreset{nullptr};

  wxTextCtrl* nameInput{nullptr};
  wxTextCtrl* dirInput{nullptr};
  wxTextCtrl* trackInput{nullptr};

  struct presetConfig {
    std::vector<wxString> styles{};
    wxString name{""};
    wxString dirs{""};
    wxString track{""};
  };
  std::vector<presetConfig> presets;

private:
  PresetsPage();
  void pushIfNewPreset();
  void rebuildPresetList();
  void rebuildBladeList();
  void resizeAndFillPresets();
  void updateFields();

  int getNumBlades();

  void stripAndSaveEditor();
  void stripAndSaveName();
  void stripAndSaveDir();
  void stripAndSaveTrack();

  bool selfEdit{false};
};
