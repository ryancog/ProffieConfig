#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/serialmonitor.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "data/primitive/models/bool.hpp"
#include "data/primitive/models/number.hpp"
#include "data/primitive/models/string.hpp"
#include "data/receiver.hpp"
#include "ui/controls/list.hpp"
#include "ui/frame.hpp"
#include "ui/types.hpp"

#include "../../tools/serialmonitor.hpp"

// This isn't actually a dialog... put it somewhere else?
struct SerialMonitorDlg : pcui::Frame, data::Receiver {
    SerialMonitorDlg(wxWindow *, std::string);

    [[nodiscard]] std::string_view device() const { return mDev; }

private:
    enum {
        eID_Input = 100,
        eID_Output,

        eID_Menu_Copy,
        eID_Menu_Copy_With_Time,
    };

    pcui::DescriptorPtr ui();

    void bindEvents();

    void onCmdChange();
    void onCmdEnter();

    void onAutoScroll();
    void doAutoScroll();

    void doClear();
    void doHelp();

    void onKey(wxKeyEvent&);
    void onUp();
    void onDown();

    void onOutputContext(wxContextMenuEvent&);
    void onOutputMenu(wxCommandEvent&);
    void doCopyOutput(bool);

    void onClose(wxCloseEvent&);

    void onDisconnect();
    void connectLoop();
    void listenLoop();

    pcui::List::Label getLabel(size, size);

    SerialMonitor mMon;
    std::string mDev;

    std::binary_semaphore mStopConnecting{0};
    std::thread mConnectThread;
    std::thread mListenThread;

    data::prim::String mInput;

    struct DeviceLine {
        data::prim::String line_;
    };

    struct UserLine {
        data::prim::String line_;
    };

    struct EventLine {
        enum class Type {


        } type_;
    };
    
    using LineVariant = std::variant<DeviceLine, UserLine, EventLine>;
    struct LineData {
        template <typename ...Args>
        LineData(std::string&& str, Args&&... args) :
            stamp_(std::move(str)), var_(std::forward<Args>(args)...) {}

        template <typename ...Args>
        LineData(Args&&... args) : var_(std::forward<Args>(args)...) {}

        data::prim::String stamp_;
        LineVariant var_;
    };

    data::prim::Bool mAutoScroll;
    data::prim::Integer mNumLines;
    std::vector<std::unique_ptr<LineData>> mLines;

    size mHistoryIdx;

    // This is accessed from the UI thread, it shouldn't need locking.
    static std::vector<std::string> smHistory;
};

