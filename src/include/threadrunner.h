#include <functional>
#include <wx/thread.h>

#pragma once

class ThreadRunner : public wxThreadHelper {
public:
  ThreadRunner(std::function<void(void)> func) {
    runFunc = func;

    CreateThread(wxTHREAD_DETACHED);
    GetThread()->Run();
  }

  wxThread::ExitCode Entry() {
    runFunc();
    return (wxThread::ExitCode)0;
  }

private:
  std::function<void(void)> runFunc;
};
