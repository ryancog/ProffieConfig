// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include <wx/wx.h>

#include "core/config/settings.h"
#include "core/utilities/progress.h"
#include "core/utilities/threadrunner.h"
#include "editor/pages/bladeidpage.h"
#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/proppage.h"

#pragma once

class EditorWindow : public wxFrame {
public:
  EditorWindow();
  static EditorWindow* instance;

  GeneralPage* generalPage{nullptr};
  PropPage* propPage{nullptr};
  BladesPage* bladesPage{nullptr};
  PresetsPage* presetsPage{nullptr};
  BladeIDPage* idPage{nullptr};
  Settings* settings{nullptr};

  ThreadRunner* thread{nullptr};
  Progress* progDialog{nullptr};

  wxBoxSizer* master{nullptr};

  wxButton* refreshButton{nullptr};
  wxButton* applyButton{nullptr};
  wxComboBox* windowSelect{nullptr};
  wxComboBox* devSelect{nullptr};

private:
  enum {
    ID_WindowSelect,
    ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in wxTextCtrl? This is a workaround.
    ID_Copyright,
    ID_Initialize,
    ID_RefreshDev,
    ID_ApplyChanges,
    ID_DeviceSelect,
    ID_Docs,
    ID_Issue,

    ID_GenFile,
    ID_VerifyConfig,
    ID_ExportFile,
    ID_ImportFile,

    ID_OpenSerial,
    ID_StyleEditor,
  };

  void bindEvents();
  void createToolTips();
  void createMenuBar();
  void createPages();
  void loadProps();
};
