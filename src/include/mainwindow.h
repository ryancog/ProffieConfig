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
  void CreatePages();

  static MainWindow* instance;
  ThreadRunner* thread;
  Progress* progDialog;

  wxComboBox* windowSelect;
  wxComboBox* devSelect;

private:
  wxBoxSizer* master;
  wxButton* refreshButton;
  wxButton* applyButton;

  GeneralPage* general;
  PropPage* prop;
  PresetsPage* presets;
  BladesPage* blades;
  HardwarePage* hardware;

  void BindEvents();
  void CreateMenuBar();
  void Initialize();
};
