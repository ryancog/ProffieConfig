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

#if defined(__WXOSX__) or defined(__WXGTK__)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#elif defined(__WXMSW__)
#include <fileapi.h>
#include <handleapi.h>
#include "log/context.h"
#else
#error Unsupported
#endif

#include <wx/gdicmn.h>
#include <wx/string.h>

#include "ui/message.h"

#include "../mainmenu/mainmenu.h"

SerialMonitor* SerialMonitor::instance;

SerialMonitor::~SerialMonitor() {
    instance = nullptr;

    if (mDevThread.joinable()) mDevThread.join();
    if (mListenThread.joinable()) mListenThread.join();
    if (mWriterThread.joinable()) mWriterThread.join();

#if defined(__WXOSX__) or defined(__WXGTK__)
    close(mFd);
#elif defined(__WXMSW__)
    CloseHandle(mSerialHandle);
#   endif
}

using namespace std::chrono_literals;

SerialMonitor::SerialMonitor(MainMenu* parent, const string& boardPath) : 
    PCUI::Frame(parent, wxID_ANY, "Proffie Serial") {
    instance = this;

    auto *master{new wxBoxSizer(wxVERTICAL)};

    mInput = new wxTextCtrl(
        this,
        ID_SerialCommand,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_PROCESS_ENTER
    );
    mOutput = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxNO_BORDER
    );
    mOutput->SetSize(wxSize{500, 200});
    auto font{mOutput->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    mOutput->SetFont(font);
    mOutput->AlwaysShowScrollbars(false, false);

    master->Add(mInput);
    master->Add(mOutput, wxSizerFlags(1).Border(wxALL, 10).Expand());

    bindEvents();
    if (not openDevice(boardPath)) {
        Destroy();
        return;
    }

    master->SetMinSize(wxSize{450, 300});
    SetSizerAndFit(master);
    Show(true);
}

const wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_INPUT(wxNewEventType());
const wxEventTypeTag<SerialMonitor::SerialDataEvent> SerialMonitor::EVT_DISCON(wxNewEventType());

void SerialMonitor::bindEvents() {
    static bool needsTime{true};
    constexpr auto TIME_FORMAT_STR{"%02u:%02u:%02u.%03u | "};
    const auto timeStrLen{2 + 1 + 2 + 1 + 2 + 1 + 3 + 3};

    Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent&) {
        const auto command = mInput->GetValue();
        mInput->Clear();

        if (mHistory.empty() or mHistory.back() != command) {
            mHistory.emplace_back(command);
        }
        mHistoryIdx = static_cast<int64>(mHistory.size());

        if (command == "clear") {
            mOutput->Clear();
        } else {
            mSendOut = command;

            if (not needsTime) {
                mOutput->AppendText('\n');
            }
            mOutput->AppendText("-------------> " + mHistory.back() + '\n');
        }
    }, ID_SerialCommand);

    mInput->Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent& evt) {
        switch (evt.GetKeyCode()) {
            case WXK_DOWN:
            case WXK_NUMPAD_DOWN:
                if (mHistoryIdx < static_cast<ssize_t>(mHistory.size())) {
                    ++mHistoryIdx;
                    if (mHistoryIdx == static_cast<ssize_t>(mHistory.size())) {
                        mInput->SetValue({});
                    } else {
                        mInput->SetValue(mHistory.at(mHistoryIdx));
                        mInput->SetInsertionPointEnd();
                    }
                } else {
                    wxBell();
                }
                break;
            case WXK_UP:
            case WXK_NUMPAD_UP:
                if (mHistoryIdx > 0) {
                    --mHistoryIdx;
                    mInput->SetValue(mHistory.at(mHistoryIdx));
                    mInput->SetInsertionPointEnd();
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
            const auto text{mOutput->GetValue()};
            const auto endPos{text.rfind('\n')};

            if (endPos != string::npos) {
                if (needsTime) {
                    mOutput->SetInsertionPoint(static_cast<int32>(endPos) + 1);
                } else {
                    mOutput->SetInsertionPoint(static_cast<int32>(endPos) + 1 + timeStrLen);
                }
            } else {
                mOutput->SetInsertionPoint(0);
            }
        } else if (evt.value == '\n') {
            needsTime = true;
            mOutput->AppendText(evt.value);
        } else if (isascii(evt.value)) {
            const auto lastPos{mOutput->GetLastPosition()};

            if (needsTime) {
                // Time length + " | " + null
                std::array<char, 12 + 3 + 1> timeBuf;
                timespec time;
                (void)clock_gettime(CLOCK_REALTIME, &time);
                const auto localTime{*localtime(&time.tv_sec)};

                (void)snprintf(
                    timeBuf.data(),
                    timeBuf.size(),
                    TIME_FORMAT_STR,
                    localTime.tm_hour,
                    localTime.tm_min,
                    localTime.tm_sec,
                    static_cast<uint32_t>(time.tv_nsec / 1000000U)
                );

                const auto textPos{mOutput->GetInsertionPoint()};
                if (lastPos == textPos) {
                    mOutput->AppendText(wxString::FromAscii(timeBuf.data()));
                } else {
                    const auto replaceLen{lastPos - textPos};
                    auto timeStr{wxString::FromAscii(timeBuf.data())};
                    mOutput->Replace(textPos + 1, textPos + 1 + replaceLen, timeStr);
                    mOutput->AppendText(timeStr.substr(replaceLen));
                }

                needsTime = false;
            }

            const auto textPos{mOutput->GetInsertionPoint()};
            if (lastPos == textPos) {
                mOutput->AppendText(wxString::FromAscii(evt.value));
            } else {
                mOutput->Replace(textPos + 1, textPos + 2, wxString::FromAscii(evt.value));
            }
        }

        if (mAutoScroll) mOutput->ShowPosition(mOutput->GetLastPosition());
        mOutput->Refresh();
    }, wxID_ANY);

    Bind(EVT_DISCON, [&](SerialDataEvent&) {
        Close(true);
    }, wxID_ANY);
}

bool SerialMonitor::openDevice(const string& boardPath) {
#   if defined(__WXOSX__) or defined(__WXGTK__)
    struct termios newtio;

    mFd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
    if (mFd < 0) {
        PCUI::showMessage(_("Could not connect to Proffieboard."), _("Serial Connection Error"), wxICON_ERROR | wxOK, GetParent());
        return false;
    }

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = (tcflag_t) NULL;
    newtio.c_lflag &= ~ICANON; /* unset canonical */
    newtio.c_cc[VTIME] = 1; /* 100 millis */

    tcflush(mFd, TCIFLUSH);
    tcsetattr(mFd, TCSANOW, &newtio);
#   elif defined(__WXMSW__)
    const auto safeBoardPath{R"(\\.\)" + boardPath};
    mSerialHandle = CreateFileA(
        safeBoardPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (mSerialHandle == INVALID_HANDLE_VALUE) {
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

    SetCommState(mSerialHandle, &dcbSerialParameters);
#   endif

    createListener();
    createWriter();

    mDevThread = std::thread{[this, boardPath]() {
#   if defined(__WXOSX__) or defined(__WXGTK__)
#   elif defined(__WXMSW__)
#   endif
        struct stat info;
        while (SerialMonitor::instance != nullptr) {
#           if defined(__WXOSX__) or defined(__WXGTK__)
            if (stat(boardPath.c_str(), &info) != 0) { // Check if device is still present
#           elif defined(__WXMSW__)
            if (not ClearCommError(mSerialHandle, nullptr, nullptr)) {
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
    mListenThread = std::thread{[this]() {
        std::array<char, 256> buf;

        while (instance != nullptr) {
#           if defined(__WXOSX__) or defined(__WXGTK__)
            auto bytesRead{read(mFd, buf.data(), buf.size() - 1)};
            if (bytesRead == -1) {
#           elif defined(__WXMSW__)
            DWORD bytesRead{};
            auto res{ReadFile(
                mSerialHandle,
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
    mWriterThread = std::thread{[&]() {
        while (instance != nullptr) {
            if (not mSendOut.empty()) {

                mSendOut.resize(255);
                mSendOut.at(mSendOut.size() - 2) = '\r';
                mSendOut.at(mSendOut.size() - 1) = '\n';

                constexpr string_view NEWLINE{"\r\n"};
#               if defined(__WXOSX__) or defined(__WXGTK__)
                write(mFd, NEWLINE.data(), NEWLINE.length());
                write(mFd, mSendOut.data(), mSendOut.length());
#               elif defined(__WXMSW__)
                WriteFile(
                    mSerialHandle,
                    NEWLINE.data(),
                    NEWLINE.length(),
                    nullptr,
                    nullptr
                );
                WriteFile(
                    mSerialHandle,
                    mSendOut.data(),
                    mSendOut.length(),
                    nullptr,
                    nullptr
                );
#               endif
            }
            mSendOut.clear();
            std::this_thread::sleep_for(1ms);
        }
    }};
}
