// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once
class EditorWindow; // Forward declaration to get around circular dependencies

#include <wx/wx.h>

#include "core/config/settings.h"
#include "editor/pages/bladeidpage.h"
#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/proppage.h"


class EditorWindow : public wxFrame {
public:
  EditorWindow();

  GeneralPage* generalPage{nullptr};
  PropPage* propPage{nullptr};
  BladesPage* bladesPage{nullptr};
  PresetsPage* presetsPage{nullptr};
  BladeIDPage* idPage{nullptr};
  Settings* settings{nullptr};

  wxBoxSizer* sizer{nullptr};

  wxComboBox* windowSelect{nullptr};

private:
  enum {
    ID_WindowSelect,
    ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in wxTextCtrl? This is a workaround.

    ID_GenFile,
    ID_VerifyConfig,

    ID_StyleEditor,
  };

  void bindEvents();
  void createToolTips();
  void createMenuBar();
  void createPages();
  void loadProps();
};
