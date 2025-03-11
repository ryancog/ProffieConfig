#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../editorwindow.h"
#include "../dialogs/customoptionsdlg.h"
#include "ui/controls.h"

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

  PCUI::Choice* board{nullptr};
  wxCheckBox* massStorage{nullptr};
  wxCheckBox* webUSB{nullptr};

  CustomOptionsDlg* customOptDlg{nullptr};
  wxButton* customOptButton{nullptr};

  PCUI::Choice* orientation{nullptr};
  PCUI::Numeric* buttons{nullptr};
  PCUI::Numeric* volume{nullptr};
  PCUI::NumericDec* clash{nullptr};
  PCUI::Numeric* pliTime{nullptr};
  PCUI::Numeric* idleTime{nullptr};
  PCUI::Numeric* motionTime{nullptr};
  PCUI::Numeric* maxLEDs{nullptr};

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
