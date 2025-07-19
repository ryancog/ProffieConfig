#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/dialog.h>

#include "../editorwindow.h"


class BladeArrayDlg : public wxDialog {
public:
  BladeArrayDlg(EditorWindow*);

private:
  EditorWindow* mParent{nullptr};

  void bindEvents();
  void createToolTips() const;

  void stripAndSaveName();

  wxStaticBoxSizer *createIDSetup(wxWindow*);
  wxStaticBoxSizer *createIDPowerSettings(wxWindow*);
  wxStaticBoxSizer *createContinuousScanSettings(wxWindow*);
  wxStaticBoxSizer *createBladeDetect(wxWindow*);
};
