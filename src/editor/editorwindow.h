// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "ui/pccombobox.h"

#include <wx/wx.h>

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;
class BladeArrayPage;
class Settings;

class EditorWindow : public wxFrame {
public:
  EditorWindow(const std::string&, wxWindow*);

  const std::string& getOpenConfig();

  GeneralPage* generalPage{nullptr};
  PropsPage* propsPage{nullptr};
  BladesPage* bladesPage{nullptr};
  PresetsPage* presetsPage{nullptr};
  BladeArrayPage* bladeArrayPage{nullptr};
  Settings* settings{nullptr};

  wxBoxSizer* sizer{nullptr};

  pcComboBox* windowSelect{nullptr};

  enum {
    ID_WindowSelect,
    ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in wxTextCtrl? This is a workaround.

    ID_SaveConfig,
    ID_ExportConfig,
    ID_VerifyConfig,

    ID_StyleEditor,
  };
private:
  void bindEvents();
  void createToolTips();
  void createMenuBar();
  void createPages();

  const std::string openConfig{};
};
