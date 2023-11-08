#include "serialmonitor.h"
#include "defines.h"
#include "mainwindow.h"
#include "misc.h"
#include <thread>

#if defined(__WXOSX__) || defined(__WXGTK__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#endif


SerialMonitor* SerialMonitor::instance;
SerialMonitor::SerialMonitor() : wxFrame(MainWindow::instance, wxID_ANY, "Proffie Serial") {
  instance = this;

  wxBoxSizer* master = new wxBoxSizer(wxVERTICAL);

  input = new wxTextCtrl(this, Misc::ID_SerialCommand, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY);

  master->Add(input, BOXITEMFLAGS);
  master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

  BindEvents();
  OpenDevice();

  SetSizerAndFit(master);
  Show(true);
}
SerialMonitor::~SerialMonitor() {
  close(fd);
  instance = nullptr;
}

wxEventTypeTag<wxCommandEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
void SerialMonitor::BindEvents() {
  Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) { sendOut = true; }, Misc::ID_SerialCommand);
  Bind(EVT_INPUT, [&](wxCommandEvent& evt) { output->AppendText(((SerialDataEvent*)&evt)->value); }, wxID_ANY);
}

void SerialMonitor::OpenDevice() {
#if !defined(__WXMSW__)
  struct termios newtsio;

  fd = open(MainWindow::instance->devSelect->GetValue().data(), O_RDWR | O_NOCTTY);
  if (fd < 0) { wxMessageBox("Could not connect to proffieboard.", "Serial Error"); SerialMonitor::instance->Close(true); return; }

  memset(&newtio, 0, sizeof(newtio));

  newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = (tcflag_t)NULL;
  newtio.c_lflag = ICANON;

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);

  CreateListener();
  CreateWriter();

  deviceThread = new ThreadRunner([&]() {
    struct stat info;
    while (SerialMonitor::instance != nullptr) {
      if (stat(MainWindow::instance->devSelect->GetValue().data(), &info) != 0) { // Check if device is still present
        SerialMonitor::instance->listenerThread->GetThread()->Kill();
        SerialMonitor::instance->writerThread->GetThread()->Kill();
        SerialMonitor::instance->Close(true);
        break;
      }
    }
  });
#endif
}

void SerialMonitor::CreateListener() {
  listenerThread = new ThreadRunner([&]() {
    int res = 0;
    char buf[255];
    while (SerialMonitor::instance != nullptr) {
      buf[res] = '\0';
      if (res != 0) {
        SerialDataEvent* event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
        wxQueueEvent(SerialMonitor::instance->GetEventHandler(), event);
      }
      res = read(SerialMonitor::instance->fd, buf, 255);
    }
  });
}

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

        write(SerialMonitor::instance->fd, "\r\n", 2);
        write(SerialMonitor::instance->fd, data.data(), 255);
        SerialMonitor::instance->sendOut = false;
      }

#if defined(__WXMSW__)
      Sleep(50);
#else
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
    }
  });
}
