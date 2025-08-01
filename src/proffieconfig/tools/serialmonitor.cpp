#include "serialmonitor.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "ui/message.h"

#include "../mainmenu/mainmenu.h"
#include "wx/gdicmn.h"
#include "wx/string.h"

#ifdef __WINDOWS__
#include <windows.h>
#include "paths/paths.h"
#else
#include <chrono>
#include <format>

#include "../core/defines.h"
#endif

SerialMonitor* SerialMonitor::instance;

SerialMonitor::~SerialMonitor() {
    instance = nullptr;

#	if defined(__WXOSX__) || defined(__WXGTK__)
    if (devThread.joinable()) devThread.join();
    if (listenThread.joinable()) listenThread.join();
    if (writerThread.joinable()) writerThread.join();

    close(fd);
#	endif
}

#if defined(__WINDOWS__)
SerialMonitor::SerialMonitor(MainMenu* parent) {
    if (parent->boardSelect->GetSelection() > 0) {
        ShellExecuteW(
            nullptr,
            nullptr,
            (Paths::binaries() / "arduino-cli.exe").c_str(),
            ("monitor -p " + parent->boardSelect->GetStringSelection() + " -c baudrate=115200").c_str(),
            nullptr,
            SW_SHOWNORMAL
        );
    } else PCUI::showMessage(_("Select board first."), _("No Board Selected"), wxOK | wxICON_ERROR, parent);
}

#elif defined(__WXOSX__) || defined(__WXGTK__)
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

using namespace std::chrono_literals;

SerialMonitor::SerialMonitor(MainMenu* parent, const string& devPath) : 
    PCUI::Frame(parent, wxID_ANY, "Proffie Serial") {
    instance = this;

    wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

    input = new wxTextCtrl(
        this,
        ID_SerialCommand,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_PROCESS_ENTER
    );
    output = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxNO_BORDER
    );
    output->SetSize(wxSize{500, 200});
    auto font{output->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    output->SetFont(font);
    output->AlwaysShowScrollbars(false, false);

    master->Add(input);
    master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

    bindEvents();
    openDevice(devPath);

    master->SetMinSize(wxSize{450, 300});
    SetSizerAndFit(master);
    Show(true);
}

wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_DISCON(wxNewEventType());

void SerialMonitor::bindEvents() {
    static bool needsTime{true};
    constexpr auto timeFormatStr{"%02u:%02u:%02u.%03u | "};
    const auto timeStrLen{2 + 1 + 2 + 1 + 2 + 1 + 3 + 3};

    Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) {
        const auto command = input->GetValue();
        input->Clear();

        if (history.empty() or history.back() != command) {
            history.emplace_back(command);
        }
        historyIdx = history.size();

        if (command == "clear") {
            output->Clear();
        } else {
            sendOut = command;

            if (not needsTime) {
                output->AppendText('\n');
            }
            output->AppendText("-------------> " + history.back() + '\n');
        }
    }, ID_SerialCommand);

    input->Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent& evt) {
        switch (evt.GetKeyCode()) {
            case WXK_DOWN:
            case WXK_NUMPAD_DOWN:
                if (historyIdx < static_cast<ssize_t>(history.size())) {
                    ++historyIdx;
                    if (historyIdx == static_cast<ssize_t>(history.size())) {
                        input->SetValue({});
                    } else {
                        input->SetValue(history.at(historyIdx));
                        input->SetInsertionPointEnd();
                    }
                } else {
                    wxBell();
                }
                break;
            case WXK_UP:
            case WXK_NUMPAD_UP:
                if (historyIdx > 0) {
                    --historyIdx;
                    input->SetValue(history.at(historyIdx));
                    input->SetInsertionPointEnd();
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
            const auto text{output->GetValue()};
            const auto endPos{text.rfind('\n')};

            if (endPos != string::npos) {
                if (needsTime) {
                    output->SetInsertionPoint(endPos + 1);
                } else {
                    output->SetInsertionPoint(endPos + 1 + timeStrLen);
                }
            } else {
                output->SetInsertionPoint(0);
            }
        } else if (evt.value == '\n') {
            needsTime = true;
            output->AppendText(evt.value);
        } else if (isascii(evt.value)) {
            const auto lastPos{output->GetLastPosition()};

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


                const auto textPos{output->GetInsertionPoint()};
                if (lastPos == textPos) {
                    output->AppendText(wxString().FromAscii(timeBuf.data()));
                } else {
                    const auto replaceLen{lastPos - textPos};
                    auto timeStr{wxString().FromAscii(timeBuf.data())};
                    output->Replace(textPos + 1, textPos + 1 + replaceLen, timeStr);
                    output->AppendText(timeStr.substr(replaceLen));
                }

                needsTime = false;
            }

            const auto textPos{output->GetInsertionPoint()};
            if (lastPos == textPos) {
                output->AppendText(wxString().FromAscii(evt.value));
            } else {
                output->Replace(textPos + 1, textPos + 2, wxString().FromAscii(evt.value));
            }
        }

        if (autoScroll) output->ShowPosition(output->GetLastPosition());
        output->Refresh();
    }, wxID_ANY);

    Bind(EVT_DISCON, [&](SerialDataEvent&) {
        Close(true);
    }, wxID_ANY);
}

void SerialMonitor::openDevice(const string& devPath) {
    struct termios newtio;

    fd = open(devPath.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        PCUI::showMessage(_("Could not connect to Proffieboard."), _("Serial Connection Error"), wxICON_ERROR | wxOK, GetParent());
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

    createListener();
    createWriter();

    devThread = std::thread{[this, devPath]() {
        struct stat info;
        while (SerialMonitor::instance != nullptr) {
            if (stat(devPath.c_str(), &info) != 0) { // Check if device is still present
                if (SerialMonitor::instance != nullptr) {
                    auto *event = new SerialDataEvent(EVT_DISCON, wxID_ANY, ' ');
                    wxQueueEvent(SerialMonitor::instance, event);
                }
                break;
            }

            std::this_thread::sleep_for(50ms);
        }
    }};
}

void SerialMonitor::createListener() {
    listenThread = std::thread{[this]() {
        int32_t res;
        std::array<char, 255> buf;

        while (instance != nullptr) {
            res = read(fd, buf.data(), buf.size());
            if (res == -1) {
                if (SerialMonitor::instance != nullptr) {
                    auto *event = new SerialDataEvent(EVT_DISCON, wxID_ANY, ' ');
                    wxQueueEvent(SerialMonitor::instance, event);
                }
                break;
            }
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

void SerialMonitor::createWriter() {
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
