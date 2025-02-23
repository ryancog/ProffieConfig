#include "tools/serialmonitor.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <string>
#include <chrono>
#include <format>

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

SerialMonitor::SerialMonitor(MainMenu* parent) : wxFrame(parent, wxID_ANY, "Proffie Serial") {
    instance = this;

    wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

    input = new pcTextCtrl(this, ID_SerialCommand, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    output = new pcTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxNO_BORDER);
    auto font{output->entry()->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    output->entry()->SetFont(font);
    output->entry()->AlwaysShowScrollbars(false, false);

    master->Add(input, BOXITEMFLAGS);
    master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

    BindEvents();
    OpenDevice();

    SetSizerAndFit(master);
    Show(true);
}


SerialMonitor::~SerialMonitor() {
#	if defined(__WXOSX__) || defined(__WXGTK__)
    close(fd);
#	elif defined(__WINDOWS__)
    CloseHandle(serHandle);
#	endif

    instance = nullptr;

    if (devThread.joinable()) devThread.join();
    if (listenThread.joinable()) listenThread.join();
    if (writerThread.joinable()) writerThread.join();
}

wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_DISCON(wxNewEventType());

void SerialMonitor::BindEvents() {
    static bool needsTime{true};
    constexpr auto timeFormatStr{"%02u:%02u:%02u.%03u | "};
    const auto timeStrLen{2 + 1 + 2 + 1 + 2 + 1 + 3 + 3};

    Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) {
        const auto command = input->entry()->GetValue();
        input->entry()->Clear();

        if (history.empty() or history.back() != command) {
            history.emplace_back(command);
        }
        historyIdx = history.size();

        if (command == "clear") {
            output->entry()->Clear();
        } else {
            sendOut = command;

            if (not needsTime) {
                output->entry()->AppendText('\n');
            }
            output->entry()->AppendText("-------------> " + history.back() + '\n');
        }
    }, ID_SerialCommand);
    input->entry()->Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent& evt) {
        switch (evt.GetKeyCode()) {
            case WXK_DOWN:
            case WXK_NUMPAD_DOWN:
                if (historyIdx < static_cast<ssize_t>(history.size())) {
                    ++historyIdx;
                    if (historyIdx == static_cast<ssize_t>(history.size())) {
                        input->entry()->SetValue({});
                    } else {
                        input->entry()->SetValue(history.at(historyIdx));
                        input->entry()->SetInsertionPointEnd();
                    }
                } else {
                    wxBell();
                }
                break;
            case WXK_UP:
            case WXK_NUMPAD_UP:
                if (historyIdx > 0) {
                    --historyIdx;
                    input->entry()->SetValue(history.at(historyIdx));
                    input->entry()->SetInsertionPointEnd();
                } else {
                    wxBell();
                }
                break;
            default:
                evt.Skip();
        }
    });
    Bind(EVT_INPUT, [&](SerialDataEvent& evt) {
        if (evt.value == '\r') {
            const auto text{output->entry()->GetValue()};
            const auto endPos{text.rfind('\n')};

            if (endPos != std::string::npos) {
                if (needsTime) {
                    output->entry()->SetInsertionPoint(endPos + 1);
                } else {
                    output->entry()->SetInsertionPoint(endPos + 1 + timeStrLen);
                }
            } else {
                output->entry()->SetInsertionPoint(0);
            }
        } else if (evt.value == '\n') {
            needsTime = true;
            output->entry()->AppendText(evt.value);
        } else if (isascii(evt.value)) {
            const auto lastPos{output->entry()->GetLastPosition()};

            if (needsTime) {
                // Time length + " | " + null
                std::array<char, 12 + 3 + 1> timeBuf;
                timespec time;
                clock_gettime(CLOCK_REALTIME, &time);
                const auto localTime{*localtime(&time.tv_sec)};

                snprintf(
                    timeBuf.data(),
                    timeBuf.size(),
                    timeFormatStr,
                    localTime.tm_hour,
                    localTime.tm_min,
                    localTime.tm_sec,
                    static_cast<uint32_t>(time.tv_nsec / 1000000U)
                );


                const auto textPos{output->entry()->GetInsertionPoint()};
                if (lastPos == textPos) {
                    output->entry()->AppendText(wxString().FromAscii(timeBuf.data()));
                } else {
                    const auto replaceLen{lastPos - textPos};
                    auto timeStr{wxString().FromAscii(timeBuf.data())};
                    output->entry()->Replace(textPos + 1, textPos + 1 + replaceLen, timeStr);
                    output->entry()->AppendText(timeStr.substr(replaceLen));
                }

                needsTime = false;
            }

            const auto textPos{output->entry()->GetInsertionPoint()};
            if (lastPos == textPos) {
                output->entry()->AppendText(wxString().FromAscii(evt.value));
            } else {
                output->entry()->Replace(textPos + 1, textPos + 2, wxString().FromAscii(evt.value));
            }
        }

        if (autoScroll) output->entry()->ShowPosition(output->entry()->GetLastPosition());
        output->entry()->Refresh();
    }, wxID_ANY);
    Bind(EVT_DISCON, [&](SerialDataEvent&) {
        SerialMonitor::instance->Close(true);
    }, wxID_ANY);
}

void SerialMonitor::OpenDevice() {
    struct termios newtio;

    fd = open(static_cast<MainMenu*>(GetParent())->boardSelect->entry()->GetStringSelection().data(), O_RDWR | O_NOCTTY);
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
            if (stat(static_cast<MainMenu*>(GetParent())->boardSelect->entry()->GetStringSelection().data(), &info) != 0) { // Check if device is still present
                auto *event = new SerialDataEvent(EVT_DISCON, wxID_ANY, ' ');
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
        std::array<char, 255> buf;

        while (instance != nullptr) {
            res = read(fd, buf.data(), buf.size());
            buf[res] = '\0';


            for (const char chr : buf) {
                if (chr == 0) break;

                auto *evt{new SerialDataEvent(EVT_INPUT, wxID_ANY, chr)};
                wxQueueEvent(GetEventHandler(), evt);
            }
            std::this_thread::sleep_for(1ms);
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
            std::this_thread::sleep_for(1ms);
        }
    }};
}
#endif
