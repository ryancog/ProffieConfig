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

  input = new wxTextCtrl(this,
                         Misc::ID_SerialCommand,
                         wxEmptyString,
                         wxDefaultPosition,
                         wxDefaultSize,
                         wxTE_PROCESS_ENTER);
  output = new wxTextCtrl(this,
                          wxID_ANY,
                          wxEmptyString,
                          wxDefaultPosition,
                          wxSize(500, 200),
                          wxTE_MULTILINE | wxTE_READONLY);

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
#endif
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
    SerialMonitor::instance->listenerThread->GetThread()->Delete();
    SerialMonitor::instance->writerThread->GetThread()->Delete();
    SerialMonitor::instance->Close(true);
  }, wxID_ANY);
}

#if defined(__WXMSW__)
void SerialMonitor::OpenDevice()
{
  auto connectErr = []() {
    wxMessageBox("Could not connect to proffieboard.\n\nAre you sure you have the correct "
                 "device selected?",
                 "Serial Error");
    SerialMonitor::instance->Close(true);
  };

  serHandle = CreateFileW(MainWindow::instance->devSelect->GetValue(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          0,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          0);
  if (serHandle == INVALID_HANDLE_VALUE) {
    connectErr();
    return;
  }

  DCB serConfig{};

  serConfig.DCBlength = sizeof(serConfig);

  if (!GetCommState(serHandle, &serConfig)) {
    connectErr();
    return;
  }

  serConfig.BaudRate = CBR_115200;
  serConfig.ByteSize = 8;
  serConfig.StopBits = ONESTOPBIT;
  serConfig.Parity = NOPARITY;

  if (!SetCommState(serHandle, &serConfig)) {
    connectErr();
    return;
  }

  deviceThread = new ThreadRunner([&]() {
    while (SerialMonitor::instance != nullptr) {
      if (!GetCommState(serHandle, &serConfig)) {
        SerialMonitor::instance->listenerThread->GetThread()->Kill();
        SerialMonitor::instance->writerThread->GetThread()->Kill();
        SerialMonitor::instance->Close(true);
        break;
      }
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
  newtio.c_lflag = ICANON;

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

#if defined(__WXOSX__) || defined(__WXGTK__)
void SerialMonitor::CreateListener()
{
  listenerThread = new ThreadRunner([&]() {
    int res = 0;
    char buf[255];
    while (!listenerThread->GetThread()->TestDestroy()) {
      buf[res] = '\0';
      if (res != 0) {
        SerialDataEvent *event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
        wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
      }
      res = read(SerialMonitor::instance->fd, buf, 255);
    }
  });
}
#elif defined(__WXMSW__)
void SerialMonitor::CreateListener()
{
  listenerThread = new ThreadRunner([&]() {
    DWORD res = 0;
    char buf[255];
    while (SerialMonitor::instance != nullptr) {
      buf[res] = '\0';
      if (res != 0) {
        SerialDataEvent *event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
        wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
      }

      ReadFile(SerialMonitor::instance->serHandle, buf, 255, &res, NULL);
    }
  });
}
#endif

#if defined(__WXOSX__) || defined(__WXGTK__)
void SerialMonitor::CreateWriter()
{
  writerThread = new ThreadRunner([&]() {
    while (!writerThread->GetThread()->TestDestroy()) {
      if (!SerialMonitor::instance->sendOut.empty()) {

        SerialMonitor::instance->sendOut.resize(255);
        SerialMonitor::instance->sendOut.at(253) = '\r';
        SerialMonitor::instance->sendOut.at(254) = '\n';

        write(SerialMonitor::instance->fd, "\r\n", 2);
        write(SerialMonitor::instance->fd, SerialMonitor::instance->sendOut.data(), 255);
        SerialMonitor::instance->sendOut.clear();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));

    }
  });
}
#elif defined(__WXMSW__)
void SerialMonitor::CreateWriter() {
  writerThread = new ThreadRunner([]() {
    std::string data;
    while (SerialMonitor::instance != nullptr) {
      if (SerialMonitor::instance->sendOut) {
        data = SerialMonitor::instance->input->GetValue().ToStdString();
        SerialMonitor::instance->input->Clear();

        data.resize(255);
        data[253] = '\r';
        data[254] = '\n';
        data[255] = '\0';

        WriteFile(SerialMonitor::instance->serHandle, data.data(), 255, NULL, NULL);
      }
    }
  });
}
#endif
