// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "serialmonitor.h"

#include "mainwindow.h"
#include "defines.h"

SerialMonitor* SerialMonitor::instance;
#if defined(__WXMSW__)
SerialMonitor::SerialMonitor() {
  if (MainWindow::instance->devSelect->GetSelection() > 0) {
    ShellExecute(NULL, NULL, TEXT(ARDUINO_PATH), std::wstring("monitor -p " + MainWindow::instance->devSelect->GetStringSelection().ToStdWstring() + " -c baudrate=115200").c_str(), NULL, true);
  } else wxMessageBox("Select board first.", "No Board Selected", wxOK | wxICON_ERROR, MainWindow::instance);
}

#elif defined(__WXOSX__) || defined(__WXGTK__)

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

SerialMonitor::SerialMonitor()
    : wxFrame(MainWindow::instance, wxID_ANY, "Proffie Serial")
{
  instance = this;

  wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

  input = new wxTextCtrl(this, ID_SerialCommand, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY);

  master->Add(input, BOXITEMFLAGS);
  master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

  BindEvents();
  OpenDevice();

  SetSizerAndFit(master);
  Show(true);
}


SerialMonitor::~SerialMonitor() {
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
      }, ID_SerialCommand);
  Bind(EVT_INPUT, [&](wxCommandEvent& evt) { output->AppendText(((SerialDataEvent*)&evt)->value); }, wxID_ANY);
  Bind(EVT_DISCON, [&](wxCommandEvent&) {
        SerialMonitor::instance->Close(true);
      }, wxID_ANY);
}

void SerialMonitor::OpenDevice()
{
  struct termios newtio;

  fd = open(MainWindow::instance->devSelect->GetValue().data(), O_RDWR | O_NOCTTY);
  if (fd < 0) {
    wxMessageBox("Could not connect to proffieboard.", "Serial Error", wxICON_ERROR | wxOK, MainWindow::instance);
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

void SerialMonitor::CreateListener()
{
  listenerThread = new ThreadRunner([&]() {
    listenerRunning = true;
    int32_t res;
    char buf[255];

    while (!listenerThread->GetThread()->TestDestroy()) {
      res = read(fd, buf, 255);

      buf[res] = '\0';
      if (res != 0) {
        SerialDataEvent *event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
        wxQueueEvent(GetEventHandler(), event);
      }

      listenerThread->GetThread()->Sleep(50);
    }

    listenerRunning = false;
  });
}

void SerialMonitor::CreateWriter()
{
  writerThread = new ThreadRunner([&]() {
    writerRunning = true;

    while (!writerThread->GetThread()->TestDestroy()) {

      if (!sendOut.empty()) {

        sendOut.resize(255);
        sendOut.at(253) = '\r';
        sendOut.at(254) = '\n';

        write(fd, "\r\n", 2);
        write(fd, sendOut.data(), 255);
      }
      sendOut.clear();
      writerThread->GetThread()->Sleep(50);
    }

    writerRunning = false;
  });
}
#endif
