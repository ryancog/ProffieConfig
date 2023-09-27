#include <wx/wx.h>

#include "generalpage.h"
#include "proppage.h"
#include "presetspage.h"
#include "bladespage.h"
#include "hardwarepage.h"
#include "progress.h"
#include "threadrunner.h"

#pragma once

class MainWindow : public wxFrame {
public:
  MainWindow();

  static MainWindow* instance;
  ThreadRunner* thread;
  Progress* progDialog;

private:
  static wxBoxSizer* master;
  static wxButton* refreshButton;
  static wxComboBox* windowSelect;
  static wxComboBox* devSelect;
  static wxButton* applyButton;

  static GeneralPage* general;
  static PropPage* prop;
  static PresetsPage* presets;
  static BladesPage* blades;
  static HardwarePage* hardware;

  void BindEvents();
  void SetConfigDefaults();
  void CreateMenuBar();
  void CreatePages();
  void Initialize();
};
