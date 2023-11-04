#include <wx/wx.h>

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

  void BindEvents();
  void CreateMenuBar();
  void Initialize();
};
