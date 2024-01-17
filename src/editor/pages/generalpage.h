// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "editor/dialogs/customoptionsdlg.h"
#include "ui/pcspinctrl.h"
#include "ui/pcspinctrldouble.h"

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>

class GeneralPage : public wxStaticBoxSizer {
public:
  GeneralPage(EditorWindow*);

  pcComboBox* board{nullptr};
  wxCheckBox* massStorage{nullptr};
  wxCheckBox* webUSB{nullptr};

  CustomOptionsDlg* customOptDlg{nullptr};
  wxButton* customOptButton{nullptr};

  pcComboBox* orientation{nullptr};
  pcSpinCtrl* buttons{nullptr};
  pcSpinCtrl* volume{nullptr};
  pcSpinCtrlDouble* clash{nullptr};
  pcSpinCtrl* pliTime{nullptr};
  pcSpinCtrl* idleTime{nullptr};
  pcSpinCtrl* motionTime{nullptr};
  pcSpinCtrl* maxLEDs{nullptr};

  wxCheckBox* volumeSave{nullptr};
  wxCheckBox* presetSave{nullptr};
  wxCheckBox* enableOLED{nullptr};
  wxCheckBox* colorSave{nullptr};
  wxCheckBox* disableColor{nullptr};
  wxCheckBox* noTalkie{nullptr};
  wxCheckBox* noBasicParsers{nullptr};
  wxCheckBox* disableDiagnosticCommands{nullptr};

  enum {
    ID_CustomOptions,
  };

private:
  EditorWindow* parent{nullptr};

  void bindEvents();
  void createToolTips();

  wxStaticBoxSizer* boardSection(wxStaticBoxSizer*);
  wxStaticBoxSizer* optionSection(wxStaticBoxSizer*);
  wxBoxSizer* rightOptions(wxStaticBoxSizer*);
  wxBoxSizer* leftOptions(wxStaticBoxSizer*);
};
