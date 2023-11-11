#include "serialmonitor.h"
#include "defines.h"
#include "mainwindow.h"
#include "misc.h"
#include <thread>

#if defined(__WXOSX__) || defined(__WXGTK__)
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#endif

SerialMonitor *SerialMonitor::instance;
SerialMonitor::SerialMonitor()
    : wxFrame(MainWindow::instance, wxID_ANY, "Proffie Serial")
{
  instance = this;

  wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

  input = new wxTextCtrl(this, Misc::ID_SerialCommand, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY);

  master->Add(input, BOXITEMFLAGS);
  master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

  BindEvents();
  OpenDevice();

  SetSizerAndFit(master);
  Show(true);
}


SerialMonitor::~SerialMonitor()
{
#if defined(__WXOSX__) || defined(__WXGTK__)
  close(fd);
#elif defined(__WXMSW__)
  CloseHandle(serHandle);
#endif
  if (listenerRunning) listenerThread->GetThread()->Delete();
  if (writerRunning) writerThread->GetThread()->Delete();
  while(listenerRunning || writerRunning) {}

  instance = nullptr;
}

wxEventTypeTag<wxCommandEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
wxEventTypeTag<wxCommandEvent> SerialMonitor::EVT_DISCON(wxNewEventType());
void SerialMonitor::BindEvents()
{
  Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) {
        sendOut = SerialMonitor::instance->input->GetValue();
        SerialMonitor::instance->input->Clear();
      }, Misc::ID_SerialCommand);
  Bind(EVT_INPUT, [&](wxCommandEvent& evt) { output->AppendText(((SerialDataEvent *) &evt)->value); }, wxID_ANY);
  Bind(EVT_DISCON, [&](wxCommandEvent&) {
        SerialMonitor::instance->Close(true);
      }, wxID_ANY);
}

#if defined(__WXMSW__)
void SerialMonitor::OpenDevice()
{
  auto connectErr = []() {
    wxMessageBox("Could not connect to proffieboard.\n\n"
                 "Are you sure you have the correct device selected?",
                 "Serial Error");
    SerialMonitor::instance->Close(true);
  };

  serHandle = CreateFileW(MainWindow::instance->devSelect->GetValue(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if (serHandle == INVALID_HANDLE_VALUE) {
    connectErr();
    return;
  }

  DCB serConfig{};

  serConfig.DCBlength = sizeof(serConfig);

  GetCommState(serHandle, &serConfig);
  serConfig.BaudRate = CBR_115200;
  serConfig.ByteSize = 8;
  serConfig.StopBits = ONESTOPBIT;
  serConfig.Parity = NOPARITY;

  if (!SetCommState(serHandle, &serConfig)) {
    connectErr();
    return;
  }

  COMMTIMEOUTS timeout{};
  timeout.ReadIntervalTimeout = 50;
  timeout.ReadTotalTimeoutConstant = 50;
  timeout.ReadTotalTimeoutMultiplier = 50;
  timeout.WriteTotalTimeoutConstant = 50;
  timeout.WriteTotalTimeoutMultiplier = 10;
  SetCommTimeouts(serHandle, &timeout);

  CreateListener();
  CreateWriter();


  deviceThread = new ThreadRunner([&]() {
    try {
      while (SerialMonitor::instance != nullptr) {
        if (!GetCommState(serHandle, &serConfig)) {
          SerialDataEvent* event = new SerialDataEvent(EVT_DISCON, wxID_ANY, "");
          wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
          break;
        }
      }

    } catch (const std::exception& e) {
      wxMessageBox(e.what());
    }
  });
}
#elif defined(__WXOSX__) || defined(__WXGTK__)
void SerialMonitor::OpenDevice()
{
  struct termios newtio;

  fd = open(MainWindow::instance->devSelect->GetValue().data(), O_RDWR | O_NOCTTY);
  if (fd < 0) {
    wxMessageBox("Could not connect to proffieboard.", "Serial Error");
    SerialMonitor::instance->Close(true);
    return;
  }

  memset(&newtio, 0, sizeof(newtio));

  newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = (tcflag_t) NULL;
  newtio.c_lflag &= ~ICANON; /* unset canonical */
  newtio.c_cc[VTIME] = 1; /* 100 millis */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);

  CreateListener();
  CreateWriter();

  deviceThread = new ThreadRunner([&]() {
    struct stat info;
    while (SerialMonitor::instance != nullptr) {
      if (stat(MainWindow::instance->devSelect->GetValue().data(), &info)
          != 0) { // Check if device is still present
        SerialDataEvent* event = new SerialDataEvent(EVT_DISCON, wxID_ANY, "");
        wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
        break;
      }
    }
  });
}
#endif

void SerialMonitor::CreateListener()
{
  listenerThread = new ThreadRunner([&]() {
    listenerRunning = true;
    char buf[255];
    while (!listenerThread->GetThread()->TestDestroy()) {
#     if defined(__WXOSX__) || defined(__WXGTK__)
      int res = 0;
      res = read(SerialMonitor::instance->fd, buf, 255);
#     elif defined(__WXMSW__)
      DWORD res = 0;
      ReadFile(SerialMonitor::instance->serHandle, buf, 255, &res, NULL);
#     endif

      buf[res] = '\0';
      if (res != 0) {
        SerialDataEvent *event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
        wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
      }

      writerThread->GetThread()->Sleep(50);
    }
    listenerRunning = false;
  });
}

void SerialMonitor::CreateWriter()
{
  writerThread = new ThreadRunner([&]() {
    writerRunning = true;

    while (!writerThread->GetThread()->TestDestroy()) {

      if (!SerialMonitor::instance->sendOut.empty()) {

        SerialMonitor::instance->sendOut.resize(255);
        SerialMonitor::instance->sendOut.at(253) = '\r';
        SerialMonitor::instance->sendOut.at(254) = '\n';

#       if defined(__WXOSX__) || defined(__WXGTK__)
        write(SerialMonitor::instance->fd, "\r\n", 2);
        write(SerialMonitor::instance->fd, SerialMonitor::instance->sendOut.data(), 255);
#       elif defined(__WXMSW__)
        DWORD output;
        wxMessageBox("Writing...");
        WriteFile(SerialMonitor::instance->serHandle, "\r\n", 2, &output, NULL);
        wxMessageBox("Wrote \\r\\n");
        WriteFile(SerialMonitor::instance->serHandle, SerialMonitor::instance->sendOut.data(), 255, &output, NULL);
        wxMessageBox("Wrote message");
#        endif
      }
      SerialMonitor::instance->sendOut.clear();

      writerThread->GetThread()->Sleep(50);
    }
    writerRunning = false;
  });
}
