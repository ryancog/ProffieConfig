// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "mainmenu/mainmenu.h"
#include "ui/pctextctrl.h"

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/filepicker.h>

class AddConfig : public wxDialog {
public:
  AddConfig(MainMenu*);
  enum {
    ID_CreateNew,
    ID_ImportExisting,

    ID_ChooseConfig,
    ID_ConfigName,
  };
private:

  MainMenu* parent{nullptr};

  wxToggleButton* createNew{nullptr};
  wxToggleButton* importExisting{nullptr};

  wxStaticText* chooseConfigText{nullptr};
  wxFilePickerCtrl* chooseConfig{nullptr};
  wxStaticText* configNameText{nullptr};
  pcTextCtrl* configName{nullptr};

  wxStaticText* invalidNameWarning{nullptr};
  wxStaticText* duplicateWarning{nullptr};
  wxStaticText* fileSelectionWarning{nullptr};

  void update();
  void createUI();
  void bindEvents();
};
