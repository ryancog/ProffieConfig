// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "ui/pcspinctrl.h"
#include "ui/pcspinctrldouble.h"

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>

class GeneralPage : public wxStaticBoxSizer
{
public:
  GeneralPage(wxWindow*);

  pcComboBox* board{nullptr};
  wxCheckBox* massStorage{nullptr};
  wxCheckBox* webUSB{nullptr};

  pcSpinCtrl* buttons;
  pcSpinCtrl* volume;
  pcSpinCtrlDouble* clash;
  pcSpinCtrl* pliTime;
  pcSpinCtrl* idleTime;
  pcSpinCtrl* motionTime;

  pcSpinCtrl* maxLEDs;

  wxCheckBox* volumeSave{nullptr};
  wxCheckBox* presetSave{nullptr};
  wxCheckBox* enableOLED{nullptr};
  wxCheckBox* colorSave{nullptr};
  wxCheckBox* disableColor{nullptr};
  wxCheckBox* noTalkie{nullptr};
  wxCheckBox* noBasicParsers{nullptr};
  wxCheckBox* disableDiagnosticCommands{nullptr};
  wxCheckBox* enableDeveloperCommands{nullptr};

private:
  EditorWindow* parent{nullptr};
  void createToolTips();

  wxStaticBoxSizer* boardSettings(wxStaticBoxSizer*);
  wxStaticBoxSizer* optionSettings(wxStaticBoxSizer*);
  wxBoxSizer* boolOptions(wxStaticBoxSizer*);
  wxBoxSizer* numOptions(wxStaticBoxSizer*);
};
