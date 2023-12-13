// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include <functional>
#include <wx/thread.h>

class ThreadRunner : public wxThreadHelper {
public:
  ThreadRunner(std::function<void(void)> func) {
    runFunc = func;

    CreateThread(wxTHREAD_DETACHED);
    GetThread()->Run();
  }

private:
  wxThread::ExitCode Entry() {
    runFunc();
    return (wxThread::ExitCode)0;
  }

private:
  std::function<void(void)> runFunc;
};
