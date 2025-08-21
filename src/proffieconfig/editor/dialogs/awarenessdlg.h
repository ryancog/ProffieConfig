#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/dialog.h>

#include "../editorwindow.h"


class BladeAwarenessDlg : public wxDialog {
public:
  BladeAwarenessDlg(EditorWindow*);

private:
  EditorWindow* mParent{nullptr};

  void bindEvents();

  wxWindow *createIDSetup(wxWindow *);
  wxWindow *createIDPowerSettings(wxWindow *);
  wxWindow *createContinuousScanSettings(wxWindow *);
  wxWindow *createBladeDetect(wxWindow *);
};
