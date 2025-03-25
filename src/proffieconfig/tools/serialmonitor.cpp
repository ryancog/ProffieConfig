#include "serialmonitor.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <string>

#include "ui/message.h"

#include "../mainmenu/mainmenu.h"

#ifdef __WINDOWS__
#include <windows.h>
#include "utils/paths.h"
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
    if (parent->boardSelect->entry()->GetSelection() > 0) {
        ShellExecuteA(nullptr, nullptr, (Paths::binaries() / "arduino-cli.exe").c_str(), (string{"monitor -p "} + parent->boardSelect->entry()->GetStringSelection().ToStdString() + " -c baudrate=115200").c_str(), nullptr, true);
    } else PCUI::showMessage("Select board first.", "No Board Selected", wxOK | wxICON_ERROR, parent);
}

#elif defined(__WXOSX__) || defined(__WXGTK__)
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

using namespace std::chrono_literals;

SerialMonitor::SerialMonitor(MainMenu* parent) : PCUI::Frame(parent, wxID_ANY, "Proffie Serial") {
    instance = this;

    wxBoxSizer *master = new wxBoxSizer(wxVERTICAL);

    input = new PCUI::Text(this, ID_SerialCommand, {}, wxTE_PROCESS_ENTER);
    output = new PCUI::Text(this, wxID_ANY, {}, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxNO_BORDER);
    output->SetSize(wxSize{500, 200});
    auto font{output->entry()->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    output->entry()->SetFont(font);
    output->entry()->AlwaysShowScrollbars(false, false);

    master->Add(input, BOXITEMFLAGS);
    master->Add(output, wxSizerFlags(1).Border(wxALL, 10).Expand());

    bindEvents();
    openDevice();

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
        Close(true);
    }, wxID_ANY);
}

void SerialMonitor::openDevice() {
    struct termios newtio;

    const auto boardPath{static_cast<MainMenu*>(GetParent())->boardSelect->entry()->GetStringSelection().ToStdString()};
    fd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        PCUI::showMessage("Could not connect to Proffieboard.", "Serial Error", wxICON_ERROR | wxOK, GetParent());
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

    devThread = std::thread{[this, boardPath]() {
        struct stat info;
        while (SerialMonitor::instance != nullptr) {
            if (stat(boardPath.c_str(), &info) != 0) { // Check if device is still present
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
