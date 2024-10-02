#include "tools/serialmonitor.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include <string>

#include "core/defines.h"
#include "mainmenu/mainmenu.h"

#ifdef __WINDOWS__
#include <windows.h>
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif

SerialMonitor* SerialMonitor::instance;
#if defined(__WINDOWS__)
SerialMonitor::SerialMonitor(MainMenu* parent) {
  if (parent->boardSelect->entry()->GetSelection() > 0) {
        ShellExecute(NULL, NULL, TEXT(ARDUINO_PATH), std::wstring(L"monitor -p " + parent->boardSelect->entry()->GetStringSelection().ToStdWstring() + L" -c baudrate=115200").c_str(), NULL, true);
  } else wxMessageDialog(parent, "Select board first.", "No Board Selected", wxOK | wxICON_ERROR).ShowModal();
}

#elif defined(__WXOSX__) || defined(__WXGTK__)

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

using namespace std::chrono_literals;

SerialMonitor::SerialMonitor(MainMenu* parent) : wxFrame(parent, wxID_ANY, "Proffie Serial")
{
  instance = this;

  wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

  input = new pcTextCtrl(this, ID_SerialCommand, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  output = new pcTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY);

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
#elif defined(__WINDOWS__)
  CloseHandle(serHandle);
#endif

  instance = nullptr;

  if (devThread.joinable()) devThread.join();
  if (listenThread.joinable()) listenThread.join();
  if (writerThread.joinable()) writerThread.join();
}

wxEventTypeTag<wxCommandEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
wxEventTypeTag<wxCommandEvent> SerialMonitor::EVT_DISCON(wxNewEventType());
void SerialMonitor::BindEvents()
{
  Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) {
        sendOut = SerialMonitor::instance->input->entry()->GetValue();
        SerialMonitor::instance->input->entry()->Clear();
      }, ID_SerialCommand);
  Bind(EVT_INPUT, [&](wxCommandEvent& evt) { output->entry()->AppendText(((SerialDataEvent*)&evt)->value); }, wxID_ANY);
  Bind(EVT_DISCON, [&](wxCommandEvent&) {
        SerialMonitor::instance->Close(true);
      }, wxID_ANY);
}

void SerialMonitor::OpenDevice() {
    struct termios newtio;

    fd = open(static_cast<MainMenu*>(GetParent())->boardSelect->entry()->GetValue().data(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        wxMessageDialog(GetParent(), "Could not connect to proffieboard.", "Serial Error", wxICON_ERROR | wxOK).ShowModal();
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

    devThread = std::thread{[this]() {
        struct stat info;
        while (SerialMonitor::instance != nullptr) {
            if (stat(static_cast<MainMenu*>(GetParent())->boardSelect->entry()->GetValue().data(), &info) != 0) { // Check if device is still present
                SerialDataEvent* event = new SerialDataEvent(EVT_DISCON, wxID_ANY, "");
                wxQueueEvent(SerialMonitor::instance, event);
                break;
            }

            std::this_thread::sleep_for(50ms);
        }
    }};
}

void SerialMonitor::CreateListener() {
    listenThread = std::thread{[this]() {
        int32_t res;
        char buf[255];

        while (instance != nullptr) {
            res = read(fd, buf, 255);

            buf[res] = '\0';
            if (res != 0) {
                SerialDataEvent *event = new SerialDataEvent(EVT_INPUT, wxID_ANY, buf);
                wxQueueEvent(GetEventHandler(), event);
            }

            std::this_thread::sleep_for(50ms);
        }
    }};
}

void SerialMonitor::CreateWriter() {
    writerThread = std::thread{[&]() {
        while (instance != nullptr) {
            if (!sendOut.empty()) {

                sendOut.resize(255);
                sendOut.at(253) = '\r';
                sendOut.at(254) = '\n';

                write(fd, "\r\n", 2);
                write(fd, sendOut.data(), 255);
            }
            sendOut.clear();
            std::this_thread::sleep_for(50ms);
        }
    }};
}
#endif
