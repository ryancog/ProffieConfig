#include "serialmonitor.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/tools/serialmonitor.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <chrono>
#include <format>

#if defined(__WXOSX__) or defined(__WXGTK__)
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#elif defined(__WXMSW__)
#include <fileapi.h>
#include <handleapi.h>
#else
#error Unsupported
#endif

#include <wx/gdicmn.h>
#include <wx/string.h>

#include "ui/message.h"
#include "log/context.h"

#include "../mainmenu/mainmenu.h"

SerialMonitor* SerialMonitor::instance;

SerialMonitor::~SerialMonitor() {
    instance = nullptr;

    if (devThread.joinable()) devThread.join();
    if (listenThread.joinable()) listenThread.join();
    if (writerThread.joinable()) writerThread.join();

#if defined(__WXOSX__) or defined(__WXGTK__)
    close(fd);
#elif defined(__WXMSW__)
    CloseHandle(serialHandle);
#   endif
}

using namespace std::chrono_literals;

SerialMonitor::SerialMonitor(MainMenu* parent, const string& boardPath) : 
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
    if (not openDevice(boardPath)) {
        Destroy();
        return;
    }

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

bool SerialMonitor::openDevice(const string& boardPath) {
#   if defined(__WXOSX__) or defined(__WXGTK__)
    struct termios newtio;

    fd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        PCUI::showMessage(_("Could not connect to Proffieboard."), _("Serial Connection Error"), wxICON_ERROR | wxOK, GetParent());
        return false;
    }

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = (tcflag_t) NULL;
    newtio.c_lflag &= ~ICANON; /* unset canonical */
    newtio.c_cc[VTIME] = 1; /* 100 millis */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
#   elif defined(__WXMSW__)
    const auto safeBoardPath{R"(\\.\)" + boardPath};
    serialHandle = CreateFileA(
        safeBoardPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (serialHandle == INVALID_HANDLE_VALUE) {
        PCUI::showMessage(_("Could not connect to Proffieboard."), _("Serial Connection Error"), wxICON_ERROR | wxOK, GetParent());
        return false;
    }

    DCB dcbSerialParameters = {};
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);

    dcbSerialParameters.BaudRate = CBR_115200;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fRtsControl = RTS_CONTROL_ENABLE;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    SetCommState(serialHandle, &dcbSerialParameters);
#   endif

    createListener();
    createWriter();

    devThread = std::thread{[this, boardPath]() {
#   if defined(__WXOSX__) or defined(__WXGTK__)
#   elif defined(__WXMSW__)
#   endif
        struct stat info;
        while (SerialMonitor::instance != nullptr) {
#           if defined(__WXOSX__) or defined(__WXGTK__)
            if (stat(boardPath.c_str(), &info) != 0) { // Check if device is still present
#           elif defined(__WXMSW__)
            if (not ClearCommError(serialHandle, nullptr, nullptr)) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::DBUG,
                    "SerialMonitor",
                    "Device closed with: " + std::to_string(GetLastError())
                );
#           endif
                if (SerialMonitor::instance != nullptr) {
                    auto *event = new SerialDataEvent(EVT_DISCON, wxID_ANY, ' ');
                    wxQueueEvent(SerialMonitor::instance, event);
                }
                break;
            }

            std::this_thread::sleep_for(50ms);
        }
    }};

    return true;
}

void SerialMonitor::createListener() {
    listenThread = std::thread{[this]() {
        std::array<char, 256> buf;

        while (instance != nullptr) {
#           if defined(__WXOSX__) or defined(__WXGTK__)
            auto bytesRead{read(fd, buf.data(), buf.size() - 1)};
            if (bytesRead == -1) {
#           elif defined(__WXMSW__)
            DWORD bytesRead{};
            auto res{ReadFile(
                serialHandle,
                buf.data(),
                buf.size() - 1,
                &bytesRead,
                nullptr
            )};
            if (not res) {
                Log::Context::getGlobal().quickLog(
                    Log::Severity::DBUG,
                    "SerialMonitor",
                    "Read failed (closing) with: " + std::to_string(GetLastError())
                );
#           endif
                if (SerialMonitor::instance != nullptr) {
                    auto *event = new SerialDataEvent(EVT_DISCON, wxID_ANY, ' ');
                    wxQueueEvent(SerialMonitor::instance, event);
                }
                break;
            }
            buf[bytesRead] = '\0';


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
            if (not sendOut.empty()) {

                sendOut.resize(255);
                sendOut.at(sendOut.size() - 2) = '\r';
                sendOut.at(sendOut.size() - 1) = '\n';

                constexpr string_view NEWLINE{"\r\n"};
#               if defined(__WXOSX__) or defined(__WXGTK__)
                write(fd, NEWLINE.data(), NEWLINE.length());
                write(fd, sendOut.data(), sendOut.length());
#               elif defined(__WXMSW__)
                WriteFile(
                    serialHandle,
                    NEWLINE.data(),
                    NEWLINE.length(),
                    nullptr,
                    nullptr
                );
                WriteFile(
                    serialHandle,
                    sendOut.data(),
                    sendOut.length(),
                    nullptr,
                    nullptr
                );
#               endif
            }
            sendOut.clear();
            std::this_thread::sleep_for(1ms);
        }
    }};
}
