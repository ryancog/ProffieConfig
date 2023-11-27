#include <wx/wx.h>

#include "progress.h"
#include "threadrunner.h"

#pragma once

class MainWindow : public wxFrame {
public:
  MainWindow();

  static MainWindow* instance;
  ThreadRunner* thread;
  Progress* progDialog;

  wxBoxSizer* master;

  wxButton* refreshButton;
  wxButton* applyButton;
  wxComboBox* windowSelect;
  wxComboBox* devSelect;

private:
  void BindEvents();
  void CreateMenuBar();
  void CreatePages();
  void Initialize();
};
